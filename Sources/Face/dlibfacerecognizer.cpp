#include "dlibfacerecognizer.h"

namespace cv { namespace oirt {

DlibFaceRecognizer::DlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, const String &_replayattackmodelfile, const String &_printattackmodelfile, DistanceType _disttype, double _threshold, double _minattackprob) :
    CNNImageRecognizer(cv::Size(0,0),NoCrop,ColorOrder::RGB,_disttype,_threshold), // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
    minattackprob(_minattackprob)
{
    try {
        dlibfacedet = dlib::get_frontal_face_detector();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_faceshapemodelfile.c_str()) >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_facedescriptormodelfile.c_str()) >> inet;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::attackdetmodel _tmpmodel;
        dlib::deserialize(_replayattackmodelfile.c_str()) >> _tmpmodel;
        ranet.subnet() = _tmpmodel.subnet();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::attackdetmodel _tmpmodel;
        dlib::deserialize(_printattackmodelfile.c_str()) >> _tmpmodel;
        panet.subnet() = _tmpmodel.subnet();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    // Define errors
    errorsInfo[1] = "Can not find face!";
    errorsInfo[2] = "Spoofing attack detected!";
}

Mat DlibFaceRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    return getImageDescription(_img,_error);
}

Mat DlibFaceRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput());

    auto _facerect = __detectbiggestface(_preprocessedmat);

    if(_facerect.area() != 0) {
        // Spoofing control
        if((minattackprob < 0.999) && spoofingcontrolenabled) {
            dlib::matrix<dlib::rgb_pixel> attack_facechip = __extractface(_preprocessedmat,_facerect,100,0.2);
            dlib::matrix<float,1,2> replay_attack_prob = dlib::mat(ranet(attack_facechip));
            dlib::matrix<float,1,2> print_attack_prob = dlib::mat(panet(attack_facechip));
            double attack_prob = std::max(replay_attack_prob(1),print_attack_prob(1)); // 1 is 'attack', 0 is 'live'
            if(attack_prob >= minattackprob) {
                if(_error)
                    *_error = 2;
                return cv::Mat::zeros(1,128,CV_32FC1);
            }
        }

        if(_error)
            *_error = 0;
        dlib::matrix<dlib::rgb_pixel> _facechip = __extractface(_preprocessedmat,_facerect,150,0.25);
        dlib::matrix<float,0,1> _facedescription = inet(_facechip);
        return dlib::toMat(_facedescription).reshape(1,1).clone();

    } else if(_error) {
        *_error = 1;
    }
    return cv::Mat::zeros(1,128,CV_32FC1);
}

void DlibFaceRecognizer::predict(InputArray src, Ptr<PredictCollector> collector, int *_error) const
{
    cv::Mat _description = getImageDescription(src.getMat(),_error);
    collector->init(v_labels.size());
    for (size_t sampleIdx = 0; sampleIdx < v_labels.size(); sampleIdx++) {
        if(v_whitelist[sampleIdx] != 0x00) { // only whitelisted values
            double distance = DBL_MAX;
            switch(getDistanceType()) {
            case DistanceType::Euclidean:
                distance = euclideanDistance(v_descriptions[sampleIdx], _description);
                break;
            case DistanceType::Cosine:
                distance =  cosineDistance(v_descriptions[sampleIdx], _description);
                break;
            }
            if( !collector->collect(v_labels[sampleIdx], distance) ) {
                return;
            }
        }
    }
}

dlib::matrix<dlib::rgb_pixel> DlibFaceRecognizer::__extractface(const Mat &_inmat, const dlib::rectangle &_facerect,  unsigned long _targetsize, double _padding) const
{
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_inmat);
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,_targetsize,_padding), _facechip);
    return _facechip;
}

dlib::rectangle DlibFaceRecognizer::__detectbiggestface(const Mat &_inmat) const
{
    dlib::rectangle _facerect;
    cv::Mat _graymat;
    cv::cvtColor(_inmat, _graymat, cv::COLOR_RGB2GRAY);
    std::vector<dlib::rectangle> _facerects = dlibfacedet(dlib::cv_image<unsigned char>(_graymat));
    if(_facerects.size() > 0) {
        if(_facerects.size() > 1) {
            std::sort(_facerects.begin(),_facerects.end(),[](const dlib::rectangle &lhs, const dlib::rectangle &rhs) {
                return lhs.area() > rhs.area();
            });
        }
        _facerect = _facerects[0];
    }
    return _facerect;
}

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, const String &_replayattackmodelfile, const String &_printattackmodelfile, DistanceType _disttype, double _threshold, double _minattackprob)
{
    return makePtr<DlibFaceRecognizer>(_faceshapemodelfile,_facedescriptormodelfile,_replayattackmodelfile,_printattackmodelfile,_disttype,_threshold,_minattackprob);
}

}
}
