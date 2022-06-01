#ifndef DOCRECOGNIZER_H
#define DOCRECOGNIZER_H

#include "cnnimageclassifier.h"

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

template <typename SUBNET> using alevel2 = ares<8*8,ares_down<8*8,SUBNET>>;
template <typename SUBNET> using alevel3 = ares<4*8,ares_down<4*8,SUBNET>>;
template <typename SUBNET> using alevel4 = ares<2*8,ares_down<2*8,SUBNET>>;

// testing network type (replaced batch normalization with fixed affine transforms)
using anet_type =   loss_multiclass_log<fc<6,avg_pool_everything<
                                        alevel2<
                                        alevel3<
                                        alevel4<
                                        max_pool<3,3,2,2,relu<affine<con<8,7,7,2,2,
                                        input_rgb_image
                                        >>>>>>>>>>;
}

namespace cv { namespace oirt {

class DocRecognizer : public CNNImageClassifier
{
public:
    DocRecognizer(const cv::String &_modelname);
    void predict(InputArray src, int &label, float &conf, int *_error=nullptr) const override;
    void predict(InputArray src, std::vector<float> &conf, int *_error=nullptr) const override;

    static Ptr<CNNImageClassifier> createDocRecognizer(const cv::String &_modelname);

private:
    mutable dlib::softmax<dlib::anet_type::subnet_type> net;
};

}}

#endif // DOCRECOGNIZER_H
