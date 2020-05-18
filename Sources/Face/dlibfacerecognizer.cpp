#include "dlibfacerecognizer.h"

namespace cv { namespace oirt {

DlibFaceRecognizer::DlibFaceRecognizer(const String &_resourcesdirectory, DistanceType _disttype, double _threshold, double _livenessthresh) :
    CNNImageRecognizer(cv::Size(0,0),NoCrop,ColorOrder::RGB,_disttype,_threshold), // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
    livenessthresh(_livenessthresh)
{
    try {
        dlibfacedet = dlib::get_frontal_face_detector();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_resourcesdirectory + "/shape_predictor_5_face_landmarks.dat") >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_resourcesdirectory + "/dlib_face_recognition_resnet_model_v1.dat") >> inet;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    anets = std::vector<dlib::softmax<dlib::attackmodel::subnet_type>>(5);
    for(size_t i = 0; i < anets.size(); ++i) {
        try {
            dlib::attackmodel _tmpnet;
            dlib::deserialize(_resourcesdirectory + "/net_0_split_" + std::to_string(i) + ".dat") >> _tmpnet;
            anets[i].subnet() = _tmpnet.subnet();
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
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
        const dlib::matrix<dlib::rgb_pixel> _facechip = __extractface(_preprocessedmat,_facerect,150,0.25);
        // Spoofing control
        if((livenessthresh < 0.9999) && spoofingcontrolenabled) {
            float live = 0;
            for(size_t i = 0; i < anets.size(); ++i) {
                dlib::matrix<float,1,4> p = dlib::mat(anets[i](_facechip));
                live += p(0);
            }
            live /= anets.size();
            //std::cout << "Confidence of liveness: " << live << std::endl;

            if(live < livenessthresh) { // spoofing detection criteria
                if(_error)
                    *_error = 2;
                return cv::Mat::zeros(1,128,CV_32FC1);
            }
        }
        if(_error)
            *_error = 0;
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

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_resourcesdirectory, DistanceType _disttype, double _threshold, double _livenessthresh)
{
    return makePtr<DlibFaceRecognizer>(_resourcesdirectory,_disttype,_threshold,_livenessthresh);
}

}
}
