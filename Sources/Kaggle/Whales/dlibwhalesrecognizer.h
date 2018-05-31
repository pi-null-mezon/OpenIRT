#ifndef DLIBWHALESRECOGNIZER_H
#define DLIBWHALESRECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

// https://www.kaggle.com/c/whale-categorization-playground

namespace dlib {
template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel0 = ares<512,ares<512,ares_down<512,SUBNET>>>;
template <typename SUBNET> using alevel1 = ares<256,ares<256,ares_down<256,SUBNET>>>;
template <typename SUBNET> using alevel2 = ares<128,ares<128,ares_down<128,SUBNET>>>;
template <typename SUBNET> using alevel3 = ares<64,ares<64,ares_down<64,SUBNET>>>;
template <typename SUBNET> using alevel4 = ares<32,ares<32,ares_down<32,SUBNET>>>;

using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            relu<affine<con<16,5,5,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;
}

namespace cv { namespace imgrec {

class DlibWhalesRecognizer: public CNNImageRecognizer
{
public:
    DlibWhalesRecognizer(const String &_descriptormodelfile, DistanceType _disttype, double _threshold);

    Mat   getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const override;
    Mat   getImageDescription(const Mat &_img) const override;
    void  predict(InputArray src, Ptr<PredictCollector> collector) const override;

private:
    mutable dlib::anet_type net;
};

Ptr<CNNImageRecognizer> createDlibWhalesRecognizer(const String &_descriptormodelfile="dlib_whales_metric_resnet.dat", DistanceType _disttype=DistanceType::Euclidean, double _threshold=DBL_MAX);

}}

#endif // DLIBWHALESRECOGNIZER_H
