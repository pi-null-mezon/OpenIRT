#include "faceageclassifier.h"

//#include <opencv2/highgui.hpp>

namespace cv { namespace oirt {

FaceAgeClassifier::FaceAgeClassifier(const cv::String &_prototextfilename, const cv::String &_caffemodelfilename, const cv::String &_dlibshapepredictor) :
    CNNImageClassifier(Size(0,0),ColorOrder::BGR,CropMethod::NoCrop)
{
    try {
        net = dnn::readNetFromCaffe(_prototextfilename,_caffemodelfilename);
    } catch (const cv::Exception &err) {
        std::cerr << err.msg << std::endl;
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

    // https://talhassner.github.io/home/projects/cnn_agegender/CVPR2015_CNN_AgeGenderEstimation.pdf
    setLabelInfo(0,"0-2 years");
    setLabelInfo(1,"4-6 years");
    setLabelInfo(2,"8-13 years");
    setLabelInfo(3,"15-20 years");
    setLabelInfo(4,"25-32 years");
    setLabelInfo(5,"38-43 years");
    setLabelInfo(6,"48-53 years");
    setLabelInfo(7,">60 years");
}

void FaceAgeClassifier::predict(InputArray src, int &label, float &conf, int *_error) const
{
    auto _dlibfacechip = __extractface(preprocessImageForCNN(src.getMat(),getInputSize(),getColorOrder(),getCropInput()));
    cv::Mat _preprocmat = dlib::toMat(_dlibfacechip);
    //cv::imshow("FaceAgeClassifier::predict",_preprocmat);
    net.setInput(dnn::blobFromImage(_preprocmat,1,Size(),cv::Scalar(104,117,123)),"data");
    Mat _probmat = net.forward("prob").reshape(1,1);
    auto maxelementiterator = std::max_element(_probmat.begin<float>(),_probmat.end<float>());
    label = maxelementiterator - _probmat.begin<float>();
    conf = *maxelementiterator;
}

void FaceAgeClassifier::predict(InputArray src, std::vector<float> &conf, int *_error) const
{
    // TO DO
}

Ptr<CNNImageClassifier> FaceAgeClassifier::createCNNImageClassifier(const String &_prototextfilename, const String &_caffemodelfilename, const String &_dlibshapepredictor)
{
    return makePtr<FaceAgeClassifier>(_prototextfilename,_caffemodelfilename,_dlibshapepredictor);
}

dlib::matrix<dlib::rgb_pixel> FaceAgeClassifier::__extractface(const Mat &_inmat) const
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
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,227,0.4), _facechip);

    return _facechip;
}

}}
