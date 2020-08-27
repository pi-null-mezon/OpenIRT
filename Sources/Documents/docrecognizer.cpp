#include "docrecognizer.h"

#include <dlib/opencv.h>
#include <opencv2/highgui.hpp>

namespace cv { namespace oirt {

DocRecognizer::DocRecognizer(const cv::String &_modelname) :
    CNNImageClassifier(Size(386,512),ColorOrder::RGB,CropMethod::NoCrop)
{
    try {
        dlib::anet_type _tmpnet;
        dlib::deserialize(_modelname) >> _tmpnet;
        net.subnet() = _tmpnet.subnet();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }    
    // Labels known to networks
    setLabelInfo(0,"Helix_Comb");
    setLabelInfo(1,"Invitro");
    setLabelInfo(2,"CMD");
    setLabelInfo(3,"Helix2");
    setLabelInfo(4,"Helix1");
    setLabelInfo(5,"KDL_Hemo_Inv");
}

void DocRecognizer::predict(InputArray src, int &label, float &conf, int *_error) const
{
    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    dlib::cv_image<dlib::rgb_pixel> _wrapimg(_preprocessedmat);
    dlib::matrix<dlib::rgb_pixel> _preprocesseddlibmatrix;
    dlib::assign_image(_preprocesseddlibmatrix,_wrapimg);
    double _tm1 = cv::getTickCount();
    dlib::matrix<float,1,6> prob = dlib::mat(net(_preprocesseddlibmatrix));
    std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
    label = dlib::index_of_max(prob);
    conf = prob(label);
    if(_error)
        *_error = 0;
}

void DocRecognizer::predict(InputArray src, std::vector<float> &conf, int *_error) const
{

    cv::Mat _preprocessedmat = preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput());
    dlib::cv_image<dlib::rgb_pixel> _wrapimg(_preprocessedmat);
    dlib::matrix<dlib::rgb_pixel> _preprocesseddlibmatrix;
    dlib::assign_image(_preprocesseddlibmatrix,_wrapimg);
    double _tm1 = cv::getTickCount();
    dlib::matrix<float,1,6> prob = dlib::mat(net(_preprocesseddlibmatrix));
    std::cout << 1000.0 * (cv::getTickCount() - _tm1) / cv::getTickFrequency() << " ms" << std::endl;
    conf.resize(dlib::num_columns(prob));
    for(long i = 0; i < dlib::num_columns(prob); ++i)
        conf[i] = prob(i);
    if(_error)
        *_error = 0;
}

Ptr<CNNImageClassifier> DocRecognizer::createDocRecognizer(const cv::String &_modelname)
{
    return makePtr<DocRecognizer>(_modelname);
}

}}
