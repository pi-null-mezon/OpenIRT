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
    // Labels known to network
    setLabelInfo(0,"live");
    setLabelInfo(1,"attack");
    // Possible errors (0 - no error)
    errorsInfo[1] = "Can not find face!";
}

void ReplayAttackDetector::predict(InputArray src, int &label, double &conf, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    auto _facerect = __detectbiggestface(_preprocessedmat);
    if(_facerect.area() != 0) {
        auto _facechip = __extractface(_preprocessedmat,_facerect,100,0.2);
        cv::imshow("facechip",dlib::toMat(_facechip));
        double _tm1 = cv::getTickCount();
        dlib::matrix<float,1,2> prob = dlib::mat(net(_facechip));
        std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
        label = dlib::index_of_max(prob);
        conf = prob(label);
        if(_error)
            *_error = 0;
    } else if(_error) {
        *_error = 1;
    }
}

void ReplayAttackDetector::predict(InputArray src, std::vector<double> &conf, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    auto _facerect = __detectbiggestface(_preprocessedmat);
    if(_facerect.area() != 0) {
        auto _facechip = __extractface(_preprocessedmat,_facerect,100,0.2);
        dlib::matrix<float,1,2> prob = dlib::mat(net(_facechip));
        conf.resize(dlib::num_columns(prob));
        for(long i = 0; i < dlib::num_columns(prob); ++i)
            conf[i] = prob(i);
    } else if(_error) {
        *_error = 1;
    }
}

Ptr<CNNImageClassifier> ReplayAttackDetector::createReplayAttackDetector(const String &_modelname, const String &_dlibshapepredictor)
{
    return makePtr<ReplayAttackDetector>(_modelname,_dlibshapepredictor);
}

dlib::matrix<dlib::rgb_pixel> ReplayAttackDetector::__extractface(const Mat &_inmat, const dlib::rectangle &_facerect,  unsigned long _targetsize, double _padding) const
{
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_inmat);
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,_targetsize,_padding), _facechip);
    return _facechip;
}

dlib::rectangle ReplayAttackDetector::__detectbiggestface(const Mat &_inmat) const
{
    dlib::rectangle _facerect;
    cv::Mat _graymat;
    cv::cvtColor(_inmat, _graymat, CV_RGB2GRAY);
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

}}
