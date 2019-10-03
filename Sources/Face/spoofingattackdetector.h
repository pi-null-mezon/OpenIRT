#ifndef SPOOFINGATTACKDETECTOR_H
#define SPOOFINGATTACKDETECTOR_H

#include "cnnimageclassifier.h"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

namespace dlib {

template <int N, template <typename> class BN, typename SUBNET>
using block  = relu<BN<con<N,3,3,1,1,relu<BN<con<4*N,1,1,1,1,SUBNET>>>>>>;

template <int N, int K, template <typename> class BN, typename SUBNET>
using dense_block2 = relu<BN<con<N,1,1,1,1, concat3<tag3,tag2,tag1,  tag3<block<K,BN,concat2<tag2,tag1, tag2<block<K,BN, tag1<SUBNET>>>>>>>>>>;

template <int N, int K, template <typename> class BN, typename SUBNET>
using dense_block3 = relu<BN<con<N,1,1,1,1, concat4<tag4,tag3,tag2,tag1, tag4<block<K,BN,concat3<tag3,tag2,tag1,  tag3<block<K,BN,concat2<tag2,tag1, tag2<block<K,BN, tag1<SUBNET>>>>>>>>>>>>>;

template <int N, int K, typename SUBNET> using adense3 = dense_block3<N,K,affine,SUBNET>;
template <int N, int K, typename SUBNET> using adense2 = dense_block2<N,K,affine,SUBNET>;

using densenet = loss_multiclass_log<fc<2,
                            avg_pool_everything<adense2<64,8,
                            avg_pool<2,2,2,2,adense3<64,8,
                            avg_pool<2,2,2,2,adense3<64,8,
                            relu<affine<con<8,5,5,2,2,
                            input_rgb_image
                            >>>>>>>>>>>;
}


namespace cv { namespace oirt {

class SpoofingAttackDetector : public CNNImageClassifier
{
public:
    SpoofingAttackDetector(const cv::String &_replayattack_modelname, const cv::String &_printattack_modelname, const cv::String &_dlibshapepredictor);
    void predict(InputArray src, int &label, float &conf, int *_error=nullptr) const override;
    void predict(InputArray src, std::vector<float> &conf, int *_error=nullptr) const override;

    static Ptr<CNNImageClassifier> createSpoofingAttackDetector(const cv::String &_replayattack_modelname, const cv::String &_printattack_modelname, const cv::String &_dlibshapepredictor="shape_predictor_5_face_landmarks.dat");

private:

    dlib::matrix<dlib::rgb_pixel> __extractface(const Mat &_inmat, const dlib::rectangle &_facerect,  unsigned long _targetsize, double _padding) const;
    dlib::rectangle __detectbiggestface(const Mat &_inmat) const;
    mutable dlib::shape_predictor dlibshapepredictor;
    mutable dlib::frontal_face_detector dlibfacedet;
    mutable dlib::softmax<dlib::densenet::subnet_type> netra, netpa;
};

}}

#endif // SPOOFINGATTACKDETECTOR_H
