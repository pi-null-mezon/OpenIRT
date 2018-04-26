#include "cnnimagerecognizer.hpp"

#include <opencv2/highgui.hpp>

namespace cv { namespace imgrec {

CNNImageRecognizer::CNNImageRecognizer(Size _inputsize, int _inputchannels, bool _cropinput, DistanceType _disttype, double _threshold) :
    ImageRecognizer(_inputsize, _inputchannels, _cropinput, _disttype, _threshold)
{
}

void CNNImageRecognizer::train(InputArrayOfArrays src, InputArray labels, bool _visualize)
{
    __train(src, labels, false, _visualize);
}

void CNNImageRecognizer::update(InputArrayOfArrays src, InputArray labels, bool _visualize)
{
    __train(src, labels, true, _visualize);
}

void CNNImageRecognizer::remove(InputArray labels)
{
    // get the label matrix
    Mat lbls = labels.getMat();

    std::vector<int> _vlabels;
    _vlabels.reserve(v_labels.size());
    std::vector<cv::Mat> _vdescriptions;
    _vdescriptions.reserve(v_descriptions.size());

    for(size_t i = 0; i < v_labels.size(); ++i) {
        bool _shouldberemoved = false;
        for(size_t j = 0; j < lbls.total(); ++j) {            
            if(lbls.at<int>((int)j) == v_labels[i]) {
                _shouldberemoved = true;
                break;
            }
        }
        if(_shouldberemoved == false) {
            _vlabels.push_back(v_labels[i]);
            _vdescriptions.push_back(v_descriptions[i]);
        }
    }
    v_labels = std::move(_vlabels);
    v_descriptions = std::move(_vdescriptions);
}

void CNNImageRecognizer::__train(InputArrayOfArrays _src, InputArray _labels, bool _preserveData, bool _visualize)
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
        v_descriptions.clear();
        v_labels.clear();
    }

    // append labels and images to the storage
    for(size_t labelIdx = 0; labelIdx < lbls.total(); labelIdx++) {
        v_labels.push_back(lbls.at<int>((int)labelIdx));
        raw[labelIdx] = preprocessImageForCNN(raw[labelIdx], getInputSize(), getInputChannels(), getCropInput());
        if(_visualize) {
            cv::imshow("CNNFaceRecognizer",raw[labelIdx]);
            cv::waitKey(1);
        }
        v_descriptions.push_back( getImageDescription( raw[labelIdx] ) );
        std::cout << "      image description for label " << *(v_labels.end()-1) << " has been memorized" << std::endl;
    }
}

void CNNImageRecognizer::load(const FileStorage &fs)
{
    readFileNodeList(fs["descriptions"],  v_descriptions);

    fs["labels"] >> v_labels;
    const FileNode& fn = fs["labelsInfo"];
    if (fn.type() == FileNode::SEQ)
    {
        _labelsInfo.clear();
        for (FileNodeIterator it = fn.begin(); it != fn.end();)
        {
            LabelInfo item;
            it >> item;
            _labelsInfo.insert(std::make_pair(item.label, item.value));
        }
    }
}

void CNNImageRecognizer::save(FileStorage &fs) const
{
    writeFileNodeList(fs, "descriptions", v_descriptions);

    fs << "labels" << v_labels;
    fs << "labelsInfo" << "[";
    for (std::map<int, String>::const_iterator it = _labelsInfo.begin(); it != _labelsInfo.end(); it++)
        fs << LabelInfo(it->first, it->second);
    fs << "]";
}

bool CNNImageRecognizer::empty() const
{
    if(v_labels.size() > 0)
        return false;
    return true;
}

int CNNImageRecognizer::nextfreeLabel() const
{
    if(empty()) {
        return 0;
    }
    return *std::max_element(v_labels.begin(), v_labels.end()) + 1;
}

}}
