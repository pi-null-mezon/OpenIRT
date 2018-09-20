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
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using res       = relu<residual<block,N,bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using res_down  = relu<residual_down<block,N,bn_con,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;
// ----------------------------------------------------------------------------------------
#define FNUM 16
template <typename SUBNET> using alevel0 = ares<FNUM*128,ares_down<FNUM*128,SUBNET>>;
template <typename SUBNET> using alevel1 = ares<FNUM*64,ares_down<FNUM*64,SUBNET>>;
template <typename SUBNET> using alevel2 = ares<FNUM*32,ares_down<FNUM*32,SUBNET>>;
template <typename SUBNET> using alevel3 = ares<FNUM*16,ares_down<FNUM*16,SUBNET>>;
template <typename SUBNET> using alevel4 = ares<FNUM*8,ares_down<FNUM*8,SUBNET>>;
template <typename SUBNET> using alevel5 = ares<FNUM*4,ares_down<FNUM*4,SUBNET>>;
template <typename SUBNET> using alevel6 = ares<FNUM*2,ares_down<FNUM*2,SUBNET>>;
// testing network type (replaced batch normalization with fixed affine transforms)
using anet_type = loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            alevel5<
                            alevel6<
                            relu<affine<con<FNUM,7,7,2,2,
                            input_rgb_image
                            >>>>>>>>>>>>;
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

Ptr<CNNImageRecognizer> createDialyzerRecognizer(const String &_modelfile="dlib_resnet_metric_dialyzer.dat", DistanceType _disttype=DistanceType::Euclidean, double _threshold=0.335);

}}

#endif // DLIBFACERECOGNIZER_H
