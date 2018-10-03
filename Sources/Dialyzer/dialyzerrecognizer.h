#ifndef DLIBFACERECOGNIZER_H
#define DLIBFACERECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <dlib/image_processing/frontal_face_detector.h>
#include <dlib/image_processing.h>
#include <dlib/opencv.h>
#include <dlib/dnn.h>

namespace dlib {
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
// ----------------------------------------------------------------------------------------
#define FNUM 32
template <typename SUBNET> using alevel1 = ares_down<FNUM*6,SUBNET>;
template <typename SUBNET> using alevel2 = ares<FNUM*5,ares_down<FNUM*5,SUBNET>>;
template <typename SUBNET> using alevel3 = ares<FNUM*4,ares_down<FNUM*4,SUBNET>>;
template <typename SUBNET> using alevel4 = ares<FNUM*3,ares_down<FNUM*3,SUBNET>>;
template <typename SUBNET> using alevel5 = ares_down<4*FNUM,SUBNET>;
template <typename SUBNET> using alevel6 = ares_down<2*FNUM,SUBNET>;
// testing network type (replaced batch normalization with fixed affine transforms)
using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel5<
                            alevel6<
                            max_pool<2,2,2,2,relu<affine<con<FNUM,5,5,1,1,
                            input_rgb_image
                            >>>>>>>>>;
}

namespace cv { namespace oirt {

class DialyzerRecognizer: public CNNImageRecognizer
{
public:
    DialyzerRecognizer(const String &_modelfile, DistanceType _disttype, double _threshold);

    Mat     getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error=0) const override;
    Mat     getImageDescription(const Mat &_img, int *_error=0) const override;
    void    predict(InputArray src, Ptr<PredictCollector> collector, int *_error=0) const override;

private:
    mutable dlib::anet_type net;
};

Ptr<CNNImageRecognizer> createDialyzerRecognizer(const String &_modelfile="dlib_resnet_metric_dialyzer.dat", DistanceType _disttype=DistanceType::Euclidean, double _threshold=0.4);

}}

#endif // DLIBFACERECOGNIZER_H
