#ifndef DLIBFACERECOGNIZER_H
#define DLIBFACERECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

// http://blog.dlib.net/2017/02/high-quality-face-recognition-with-deep.html

namespace dlib {
    // Face identification model
    template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

    template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
    using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

    template <int N, template <typename> class BN, int stride, typename SUBNET>
    using rblck  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

    template <int N, typename SUBNET> using ares      = relu<residual<rblck,N,affine,SUBNET>>;
    template <int N, typename SUBNET> using ares_down = relu<residual_down<rblck,N,affine,SUBNET>>;

    template <typename SUBNET> using alevel0 = ares_down<256,SUBNET>;
    template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
    template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
    template <typename SUBNET> using alevel3 = ares<64,ares<64,ares<64,ares_down<64,SUBNET>>>>;
    template <typename SUBNET> using alevel4 = ares<32,ares<32,ares<32,SUBNET>>>;

    using faceidentitymodel = loss_metric<fc_no_bias<128,avg_pool_everything<
                                alevel0<
                                alevel1<
                                alevel2<
                                alevel3<
                                alevel4<
                                max_pool<3,3,2,2,relu<affine<con<32,7,7,2,2,
                                input_rgb_image_sized<150>
                                >>>>>>>>>>>>;
    // Replay attack detector
    template <int N, template <typename> class BN, typename SUBNET>
    using block  = relu<BN<con<N,3,3,1,1,relu<BN<con<4*N,1,1,1,1,SUBNET>>>>>>;

    template <int N, int K, template <typename> class BN, typename SUBNET>
    using dense_block2 = relu<BN<con<N,1,1,1,1, concat3<tag3,tag2,tag1,  tag3<block<K,BN,concat2<tag2,tag1, tag2<block<K,BN, tag1<SUBNET>>>>>>>>>>;

    template <int N, int K, template <typename> class BN, typename SUBNET>
    using dense_block3 = relu<BN<con<N,1,1,1,1, concat4<tag4,tag3,tag2,tag1, tag4<block<K,BN,concat3<tag3,tag2,tag1,  tag3<block<K,BN,concat2<tag2,tag1, tag2<block<K,BN, tag1<SUBNET>>>>>>>>>>>>>;

    template <int N, int K, typename SUBNET> using adense3 = dense_block3<N,K,affine,SUBNET>;
    template <int N, int K, typename SUBNET> using adense2 = dense_block2<N,K,affine,SUBNET>;

    using attackdetmodel = loss_multiclass_log<fc<2,
                                avg_pool_everything<adense2<64,8,
                                avg_pool<2,2,2,2,adense3<64,8,
                                avg_pool<2,2,2,2,adense3<64,8,
                                relu<affine<con<8,5,5,2,2,
                                input_rgb_image
                                >>>>>>>>>>>;
}

namespace cv { namespace oirt {

class DlibFaceRecognizer: public CNNImageRecognizer
{
public:
    DlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, const String &_replayattackmodelfile, const String &_printattackmodelfile, DistanceType _disttype, double _threshold, double _minattackprob);

    Mat     getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error=0) const override;
    Mat     getImageDescription(const Mat &_img, int *_error=0) const override;
    void    predict(InputArray src, Ptr<PredictCollector> collector, int *_error=0) const override;

private:
    dlib::matrix<dlib::rgb_pixel> __extractface(const Mat &_inmat, const dlib::rectangle &_facerect, unsigned long _targetsize, double _padding) const;
    dlib::rectangle               __detectbiggestface(const Mat &_inmat) const;

    mutable dlib::shape_predictor dlibshapepredictor;
    mutable dlib::frontal_face_detector dlibfacedet;
    mutable dlib::faceidentitymodel inet;

    mutable dlib::softmax<dlib::attackdetmodel::subnet_type> ranet, panet;
    double  minattackprob;
};

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_faceshapemodelfile="shape_predictor_5_face_landmarks.dat",
                                                 const String &_facedescriptormodelfile="dlib_face_recognition_resnet_model_v1.dat",
                                                 const String &_replayattackmodelfile="replay_attack_net_v5.dat",
                                                 const String &_printattackmodelfile="print_attack_net_v6.dat",
                                                 DistanceType _disttype=DistanceType::Euclidean,
                                                 double _threshold=0.485,
                                                 double _minattackprob=0.5);

}}

#endif // DLIBFACERECOGNIZER_H
