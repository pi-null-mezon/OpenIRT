#ifndef DLIBWHALESRECOGNIZER_H
#define DLIBWHALESRECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

// https://www.kaggle.com/c/whale-categorization-playground

namespace dlib {

#define FNUM 32

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<max_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using res       = relu<residual<block,N,bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using res_down  = relu<residual_down<block,N,bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel1 = ares_down<16*FNUM,SUBNET>;
template <typename SUBNET> using alevel2 = ares<8*FNUM,ares_down<8*FNUM,SUBNET>>;
template <typename SUBNET> using alevel3 = ares<4*FNUM,ares<4*FNUM,ares_down<4*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel4 = ares<2*FNUM,ares<2*FNUM,ares<2*FNUM,ares_down<2*FNUM,SUBNET>>>>;

// testing network type (replaced batch normalization with fixed affine transforms)
using anet_type = loss_metric<fc_no_bias<64,avg_pool_everything<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            max_pool<3,3,2,2,relu<affine<con<FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;
}

namespace cv { namespace oirt {

class DlibWhalesRecognizer: public CNNImageRecognizer
{
public:
    DlibWhalesRecognizer(const String &_descriptormodelfile, DistanceType _disttype, double _threshold);

    Mat   getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error=nullptr) const override;
    Mat   getImageDescription(const Mat &_img, int *_error=nullptr) const override;
    void  predict(InputArray src, Ptr<PredictCollector> collector, int *_error=nullptr) const override;

private:
    mutable dlib::anet_type net;
};

Ptr<CNNImageRecognizer> createDlibWhalesRecognizer(const String &_descriptormodelfile="dlib_whales_metric_resnet.dat", DistanceType _disttype=DistanceType::Euclidean, double _threshold=DBL_MAX);

dlib::matrix<float> cvmat2dlibmatrix(const cv::Mat &_cvmat);

}}

#endif // DLIBWHALESRECOGNIZER_H
