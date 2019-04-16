#ifndef FACEAGECLASSIFIER_H
#define FACEAGECLASSIFIER_H

#include "cnnimageclassifier.h"

#include <opencv2/dnn.hpp>

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

namespace cv { namespace oirt {

class FaceAgeClassifier : public CNNImageClassifier
{
public:
    FaceAgeClassifier(const cv::String &_prototextfilename, const cv::String &_caffemodelfilename, const cv::String &_dlibshapepredictor);
    void predict(InputArray src, int &label, double &conf) const override;
    void predict(InputArray src, std::vector<double> &conf) const override;

    static Ptr<CNNImageClassifier> createCNNImageClassifier(const cv::String &_prototextfilename="deploy_age.prototxt", const cv::String &_caffemodelfilename="age_net.caffemodel", const cv::String &_dlibshapepredictor="shape_predictor_5_face_landmarks.dat");

private:
    dlib::matrix<dlib::rgb_pixel> __extractface(const Mat &_inmat) const;

    mutable dlib::shape_predictor dlibshapepredictor;
    mutable dlib::frontal_face_detector dlibfacedet;
    mutable cv::dnn::Net net;
};

}}

#endif // FACEAGECLASSIFIER_H
