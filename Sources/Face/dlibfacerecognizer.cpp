#include "dlibfacerecognizer.h"

namespace cv { namespace oirt {

DlibFaceRecognizer::DlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(0,0),NoCrop,ColorOrder::RGB,_disttype,_threshold) // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
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
        dlib::deserialize(_facedescriptormodelfile.c_str()) >> net;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    // Declare errors
    errorsInfo[1] = "Can not find face!";
}

Mat DlibFaceRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    cv::String _str = _blobname; // to suppress 'unused variable' compiler warning
    dlib::matrix<dlib::rgb_pixel> _facechip = __extractface(preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput()));

    /*cv::Mat _viewmat(num_rows(_facechip), num_columns(_facechip), CV_8UC3, image_data(_facechip));
    cv::imshow("Input of DLIB",_viewmat);
    cv::waitKey(1);*/

    if(_facechip.size() != 0) {
        dlib::matrix<float,0,1> _facedescription = net(_facechip);
        return dlib::toMat(_facedescription).reshape(1,1).clone();
    } else if(_error != 0) {
        *_error = 1; // you can find error description declaration in the constructor
    }
    return cv::Mat::zeros(1,128,CV_32FC1);
}

Mat DlibFaceRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    return getImageDescriptionByLayerName(_img,cv::String(),_error);
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

dlib::matrix<dlib::rgb_pixel> DlibFaceRecognizer::__extractface(const Mat &_inmat) const
{
    cv::Mat _rgbmat = _inmat;
    cv::Mat _graymat;
    cv::cvtColor(_rgbmat, _graymat, CV_RGB2GRAY);

    dlib::cv_image<unsigned char> _graycv_image(_graymat);
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_rgbmat);

    dlib::rectangle _facerect(_inmat.cols,_inmat.rows);
    std::vector<dlib::rectangle> _facerects = dlibfacedet(_graycv_image);

    dlib::matrix<dlib::rgb_pixel> _facechip;

    if(_facerects.size() > 0) {
        if(_facerects.size() > 1) {
            std::sort(_facerects.begin(),_facerects.end(),[](const dlib::rectangle &lhs, const dlib::rectangle &rhs) {
                return lhs.area() > rhs.area();
            });
        }
        _facerect = _facerects[0];
        auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
        dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,150,0.25), _facechip);
    }    

    return _facechip;
}

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, DistanceType _disttype, double _threshold)
{
    return makePtr<DlibFaceRecognizer>(_faceshapemodelfile,_facedescriptormodelfile,_disttype,_threshold);
}

}
}
