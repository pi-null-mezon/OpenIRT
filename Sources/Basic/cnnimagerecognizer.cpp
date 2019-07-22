#include "cnnimagerecognizer.hpp"

//#include <opencv2/highgui.hpp>

namespace cv { namespace oirt {

CNNImageRecognizer::CNNImageRecognizer(Size _inputsize, CropMethod _cropinput, ColorOrder _colororder, DistanceType _disttype, double _threshold) :
    ImageRecognizer(_inputsize, _cropinput, _colororder, _disttype, _threshold),
    spoofingcontrolenabled(true)
{
}

CNNImageRecognizer::~CNNImageRecognizer()
{
}

void CNNImageRecognizer::train(InputArrayOfArrays src, InputArray labels, bool _visualize)
{
    __train(src, labels, false, _visualize);
}

void CNNImageRecognizer::update(InputArrayOfArrays src, InputArray labels, bool _visualize, int *_error)
{
    __train(src, labels, true, _visualize, _error);
}

void CNNImageRecognizer::addKnownDescription(const Mat &_dscrmat, int _label)
{
    v_labels.push_back(_label);
    v_descriptions.push_back(_dscrmat);
    v_whitelist.push_back(0x01); // 0x00 - not in the white list, all values greater than 0x01 - in the list
}

int CNNImageRecognizer::remove(InputArray labels)
{
    int _removed = 0;
    // get the label matrix
    Mat lbls = labels.getMat();

    std::vector<int> _vlabels;
    _vlabels.reserve(v_labels.size());
    std::vector<cv::Mat> _vdescriptions;
    _vdescriptions.reserve(v_descriptions.size());
    std::vector<uchar> _whitelist;
    _whitelist.reserve(v_whitelist.size());

    for(size_t i = 0; i < v_labels.size(); ++i) {
        bool _shouldberemoved = false;
        for(size_t j = 0; j < lbls.total(); ++j) {            
            if(lbls.at<int>(static_cast<int>(j)) == v_labels[i]) {
                _shouldberemoved = true;               
                break;
            }
        }
        if(_shouldberemoved == false) {
            _vlabels.push_back(v_labels[i]);
            _vdescriptions.push_back(v_descriptions[i]);
            _whitelist.push_back(v_whitelist[i]);
        } else {
            labelsInfo.erase(v_labels[i]);
            _removed++;
        }
    }
    v_labels = std::move(_vlabels);
    v_descriptions = std::move(_vdescriptions);
    v_whitelist = std::move(_whitelist);
    return _removed;
}

void CNNImageRecognizer::__train(InputArrayOfArrays _src, InputArray _labels, bool _preserveData, bool _visualize, int *_error)
{
    if(_src.kind() != _InputArray::STD_VECTOR_MAT && _src.kind() != _InputArray::STD_VECTOR_VECTOR) {
        String error_message = "The images are expected as InputArray::STD_VECTOR_MAT (a std::vector<Mat>) or _InputArray::STD_VECTOR_VECTOR (a std::vector< std::vector<...> >).";
        CV_Error(Error::StsBadArg, error_message);
    }
    if(_src.total() == 0) {
        String error_message = format("Empty training data was given. You'll need more than one sample to learn a model.");
        CV_Error(Error::StsUnsupportedFormat, error_message);
    } else if(_labels.getMat().type() != CV_32SC1) {
        String error_message = format("Labels must be given as integers (CV_32SC1). Expected %d, but was %d.", CV_32SC1, _labels.type());
        CV_Error(Error::StsUnsupportedFormat, error_message);
    }
    // get the vector of matrices
    std::vector<Mat> raw;
    _src.getMatVector(raw);
    // get the label matrix
    Mat lbls = _labels.getMat();
    // check if data is well-aligned
    if(lbls.total() != raw.size()) {
        String error_message = format("The number of samples (src) must equal the number of labels (labels). Was len(samples)=%d, len(labels)=%d.", raw.size(), lbls.total());
        CV_Error(Error::StsBadArg, error_message);
    }

    // if this model should be trained without preserving old data, delete old model data
    if(_preserveData == false) {
        clear();
        v_labels.reserve(lbls.total());
        v_descriptions.reserve(lbls.total());
        v_whitelist.reserve(lbls.total());
    } else if(v_labels.size() == 0) { // let's reserve memory if there are no labels have been remembered yet
        v_labels.reserve(1024);
        v_descriptions.reserve(1024);
        v_whitelist.reserve(1024);
    }

    // append labels and images to the storage
    for(size_t labelIdx = 0; labelIdx < lbls.total(); labelIdx++) {             
        /*if(_visualize) {
            cv::Mat _tmpmat = preprocessImageForCNN(raw[labelIdx], getInputSize(), getColorOrder(), getCropInput());
            cv::imshow("CNNImageRecognizer::__train",_tmpmat);
            cv::waitKey(1);
        }*/
        spoofingcontrolenabled = false;
        cv::Mat _dscrmat = getImageDescription(raw[labelIdx], _error);
        spoofingcontrolenabled = true;
        if(_error != nullptr) {
            if(*_error != 0) {
                return;
            }
        }
        v_labels.push_back(lbls.at<int>(static_cast<int>(labelIdx)));
        v_descriptions.push_back(_dscrmat);
        v_whitelist.push_back(0x01); // 0x00 - not in the white list, all values greater than 0x01 - in the list
    }
}

void CNNImageRecognizer::load(const FileStorage &fs)
{
    clear();
    readFileNodeList(fs["descriptions"],  v_descriptions);

    fs["labels"] >> v_labels;
    fs["whitelist"] >> v_whitelist;
    if(v_whitelist.size() < v_labels.size()) {
        v_whitelist = std::vector<uchar>(v_labels.size(),0x01);
    }
    const FileNode& fn = fs["labelsInfo"];
    if (fn.type() == FileNode::SEQ)
    {
        labelsInfo.clear();
        for (FileNodeIterator it = fn.begin(); it != fn.end();)
        {
            LabelInfo item;
            it >> item;
            labelsInfo.insert(std::make_pair(item.label, item.value));
        }
    }
}

void CNNImageRecognizer::save(FileStorage &fs) const
{
    writeFileNodeList(fs, "descriptions", v_descriptions);

    fs << "labels" << v_labels;
    fs << "whitelist" << v_whitelist;
    fs << "labelsInfo" << "[";
    for (std::map<int, String>::const_iterator it = labelsInfo.begin(); it != labelsInfo.end(); it++)
        fs << LabelInfo(it->first, it->second);
    fs << "]";
}

void CNNImageRecognizer::clear()
{
    v_labels.clear();
    v_descriptions.clear();
    v_whitelist.clear();
}

bool CNNImageRecognizer::empty() const
{
    if(v_labels.size() > 0)
        return false;
    return true;
}

bool CNNImageRecognizer::emptyWhitelist() const
{
    if(v_whitelist.size() > 0) {
        for(size_t i = 0; i < v_whitelist.size(); ++i) {
            if(v_whitelist[i] != 0x00)
                return false;
        }
    }
    return true;
}

int CNNImageRecognizer::nextfreeLabel() const
{
    if(empty()) {
        return 0;
    }
    return *std::max_element(v_labels.begin(), v_labels.end()) + 1;
}

void CNNImageRecognizer::setWhitelist(const std::vector<String> &_vlabelinfo)
{
    // Drop old whitelist data
    v_whitelist = std::vector<uchar>(v_labels.size(),0x00);
    for(size_t i = 0; i < _vlabelinfo.size(); ++i) {
        std::vector<int> _vlbls = getLabelsByString(_vlabelinfo[i]);
        if(_vlbls.size() > 0) { // so, recognizer knows this labelinfo
            for(size_t j = 0; j < v_labels.size(); ++j) {
                if(v_labels[j] == _vlbls[0])
                    v_whitelist[j] = 0x01;
            }
        }
    }
}

void CNNImageRecognizer::dropWhitelist()
{
    for(size_t i = 0; i < v_whitelist.size(); ++i) {
        v_whitelist[i] = 0x01;
    }
}

int CNNImageRecognizer::labelTemplates(int _label) const
{
    int _templates = 0;
    for(size_t i = 0; i < v_labels.size(); ++i) {
        if(v_labels[i] == _label) {
            _templates++;
        }
    }
    return _templates;
}

bool CNNImageRecognizer::isLabelWhitelisted(int _label) const
{
    bool _whitelisted = false;
    for(size_t i = 0; i < v_labels.size(); ++i) {
        if(v_labels[i] == _label) {
            if(v_whitelist[i] != 0x00) {
                _whitelisted = true;
                break;
            }
        }
    }
    return _whitelisted;
}

}}
