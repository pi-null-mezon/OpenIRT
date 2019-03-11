#include "replayattackdetector.h"

#include <opencv2/highgui.hpp>

namespace cv { namespace oirt {

ReplayAttackDetector::ReplayAttackDetector(const cv::String &_modelname, const cv::String &_dlibshapepredictor) :
    CNNImageClassifier(Size(0,0),ColorOrder::RGB,CropMethod::NoCrop)
{
    try {
        dlib::densenet _tmpnet;
        dlib::deserialize(_modelname) >> _tmpnet;
        net.subnet() = _tmpnet.subnet();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlibfacedet = dlib::get_frontal_face_detector();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_dlibshapepredictor.c_str()) >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    setLabelInfo(0,"live");
    setLabelInfo(1,"replay attack");
}

void ReplayAttackDetector::predict(InputArray src, int &label, double &conf) const
{
    auto _facechip = __extractface(preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput()));
    cv::imshow("facechip",dlib::toMat(_facechip));
    double _tm1 = cv::getTickCount();
    dlib::matrix<float,1,2> prob = dlib::mat(net(_facechip));
    std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
    label = dlib::index_of_max(prob);
    conf = prob(label);
}

Ptr<CNNImageClassifier> ReplayAttackDetector::createReplayAttackDetector(const String &_modelname, const String &_dlibshapepredictor)
{
    return makePtr<ReplayAttackDetector>(_modelname,_dlibshapepredictor);
}

dlib::matrix<dlib::rgb_pixel> ReplayAttackDetector::__extractface(const Mat &_inmat) const
{
    cv::Mat _rgbmat = _inmat;
    cv::Mat _graymat;
    cv::cvtColor(_rgbmat, _graymat, CV_BGR2GRAY);

    dlib::cv_image<unsigned char> _graycv_image(_graymat);
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_rgbmat);

    dlib::rectangle _facerect(_inmat.cols,_inmat.rows);
    std::vector<dlib::rectangle> _facerects = dlibfacedet(_graycv_image);
    if(_facerects.size() > 0) {
        if(_facerects.size() > 1) {
            std::sort(_facerects.begin(),_facerects.end(),[](const dlib::rectangle &lhs, const dlib::rectangle &rhs) {
                return lhs.area() > rhs.area();
            });
        }
        _facerect = _facerects[0];
    }
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,100,0.2), _facechip);

    return _facechip;
}

}}
