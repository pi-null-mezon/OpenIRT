#ifndef REPLAYATTACKDETECTOR_H
#define REPLAYATTACKDETECTOR_H

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

class ReplayAttackDetector : public CNNImageClassifier
{
public:
    ReplayAttackDetector(const cv::String &_modelname, const cv::String &_dlibshapepredictor);
    void predict(InputArray src, int &label, double &conf) const override;
    void predict(InputArray src, std::vector<double> &conf) const override;

    static Ptr<CNNImageClassifier> createReplayAttackDetector(const cv::String &_modelname="replayattack_v1.dat", const cv::String &_dlibshapepredictor="shape_predictor_5_face_landmarks.dat");

private:
    dlib::matrix<dlib::rgb_pixel> __extractface(const Mat &_inmat) const;
	
    mutable dlib::shape_predictor dlibshapepredictor;
    mutable dlib::frontal_face_detector dlibfacedet;
    mutable dlib::softmax<dlib::densenet::subnet_type> net;
};

}}

#endif // REPLAYATTACKDETECTOR_H
