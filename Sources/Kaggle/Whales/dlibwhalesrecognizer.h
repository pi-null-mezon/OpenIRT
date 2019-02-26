#ifndef DLIBWHALESRECOGNIZER_H
#define DLIBWHALESRECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <dlib/opencv.h>
#include <dlib/dnn.h>

// https://www.kaggle.com/c/whale-categorization-playground

namespace dlib {

#define FNUM 16

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual = add_prev1<block<N,BN,1,tag1<SUBNET>>>;

template <template <int,template<typename>class,int,typename> class block, int N, template<typename>class BN, typename SUBNET>
using residual_down = add_prev2<avg_pool<2,2,2,2,skip1<tag2<block<N,BN,2,tag1<SUBNET>>>>>>;

template <int N, template <typename> class BN, int stride, typename SUBNET>
using block  = BN<con<N,3,3,1,1,relu<BN<con<N,3,3,stride,stride,SUBNET>>>>>;

template <int N, typename SUBNET> using ares      = relu<residual<block,N,affine,SUBNET>>;
template <int N, typename SUBNET> using ares_down = relu<residual_down<block,N,affine,SUBNET>>;

template <typename SUBNET> using alevel  = ares<64*FNUM,ares_down<64*FNUM,SUBNET>>;
template <typename SUBNET> using alevel0 = ares<32*FNUM,ares_down<32*FNUM,SUBNET>>;
template <typename SUBNET> using alevel1 = ares<16*FNUM,ares_down<16*FNUM,SUBNET>>;
template <typename SUBNET> using alevel2 = ares<8*FNUM,ares_down<8*FNUM,SUBNET>>;
template <typename SUBNET> using alevel3 = ares<4*FNUM,ares_down<4*FNUM,SUBNET>>;
template <typename SUBNET> using alevel4 = ares<2*FNUM,ares_down<2*FNUM,SUBNET>>;

template <typename SUBNET> using alevel0_d = ares<32*FNUM,ares<32*FNUM,ares_down<32*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel1_d = ares<16*FNUM,ares<16*FNUM,ares_down<16*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel2_d = ares<8*FNUM,ares<8*FNUM,ares<8*FNUM,ares_down<8*FNUM,SUBNET>>>>;
template <typename SUBNET> using alevel3_d = ares<4*FNUM,ares<4*FNUM,ares<4*FNUM,ares<4*FNUM,ares_down<4*FNUM,SUBNET>>>>>;
template <typename SUBNET> using alevel4_d = ares<2*FNUM,ares<2*FNUM,ares_down<2*FNUM,SUBNET>>>;

template <typename SUBNET> using alevel2_ex = ares<8*FNUM,ares<8*FNUM,ares_down<8*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel3_ex = ares<4*FNUM,ares<4*FNUM,ares<4*FNUM,ares_down<4*FNUM,SUBNET>>>>;
template <typename SUBNET> using alevel4_ex = ares<2*FNUM,ares<2*FNUM,ares_down<2*FNUM,SUBNET>>>;

template <typename SUBNET> using alevel0_ex2 = ares<32*FNUM,ares<32*FNUM,ares_down<32*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel1_ex2 = ares<16*FNUM,ares<16*FNUM,ares_down<16*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel2_ex2 = ares<8*FNUM,ares<8*FNUM,ares_down<8*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel3_ex2 = ares<4*FNUM,ares<4*FNUM,ares<4*FNUM,ares_down<4*FNUM,SUBNET>>>>;
template <typename SUBNET> using alevel4_ex2 = ares<2*FNUM,ares<2*FNUM,ares<2*FNUM,ares_down<2*FNUM,SUBNET>>>>;

template <typename SUBNET> using alevel0_v = ares<32*FNUM,ares_down<32*FNUM,SUBNET>>;
template <typename SUBNET> using alevel1_v = ares<16*FNUM,ares<16*FNUM,ares_down<16*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel2_v = ares<8*FNUM,ares<8*FNUM,ares<8*FNUM,ares_down<8*FNUM,SUBNET>>>>;
template <typename SUBNET> using alevel3_v = ares<4*FNUM,ares<4*FNUM,ares_down<4*FNUM,SUBNET>>>;
template <typename SUBNET> using alevel4_v = ares<2*FNUM,ares_down<2*FNUM,SUBNET>>;


using resnet16 =  loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            alevel4<
                            relu<affine<con<FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;

using resnet32 =  loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel<
                            alevel0<
                            alevel1<
                            alevel2<
                            alevel3<
                            relu<affine<con<2*FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;

using exresnet16 = loss_metric<fc_no_bias<128,avg_pool_everything<
                                alevel0<
                                alevel1<
                                alevel2_ex<
                                alevel3_ex<
                                alevel4_ex<
                                relu<affine<con<2*FNUM,7,7,2,2,
                                input<matrix<float>>
                                >>>>>>>>>>>;

using ex2resnet16 =  loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0_ex2<
                            alevel1_ex2<
                            alevel2_ex2<
                            alevel3_ex2<
                            alevel4_ex2<
                            relu<affine<con<FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;

using vresnet16 =  loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0_v<
                            alevel1_v<
                            alevel2_v<
                            alevel3_v<
                            alevel4_v<
                            relu<affine<con<FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;

using dresnet16 =  loss_metric<fc_no_bias<128,avg_pool_everything<
                            alevel0_d<
                            alevel1_d<
                            alevel2_d<
                            alevel3_d<
                            alevel4_d<
                            relu<affine<con<FNUM,7,7,2,2,
                            input<matrix<float>>
                            >>>>>>>>>>>;


using headnet = loss_metric<fc_no_bias<512,
                              relu<affine<fc<512,
                              relu<affine<fc<512,
                              input<matrix<float>>>>>>>>>>;
}

namespace cv { namespace oirt {

class DlibWhalesRecognizer: public CNNImageRecognizer
{
public:
    DlibWhalesRecognizer(const String &_modelsfiles, DistanceType _disttype, double _threshold);

    Mat   getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error=nullptr) const override;
    Mat   getImageDescription(const Mat &_img, int *_error=nullptr) const override;
    void  predict(InputArray src, Ptr<PredictCollector> collector, int *_error=nullptr) const override;

private:
    mutable std::vector<dlib::resnet16> rn16;
    mutable std::vector<dlib::ex2resnet16> ex2rn16;
    mutable std::vector<dlib::dresnet16> drn16;
    mutable dlib::exresnet16 exrn16;
    mutable dlib::resnet32 rn32;
    mutable dlib::headnet headnet;
    mutable cv::RNG cvrng;
};

Ptr<CNNImageRecognizer> createDlibWhalesRecognizer(const String &_modelsfiles="dlib_whales_metric_resnet.dat", DistanceType _disttype=DistanceType::Euclidean, double _threshold=DBL_MAX);

dlib::matrix<float> cvmat2dlibmatrix(const cv::Mat &_cvmat);

}}

#endif // DLIBWHALESRECOGNIZER_H
