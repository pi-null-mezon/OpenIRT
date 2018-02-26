#ifndef RESNET50IMAGENETRECOGNIZER_H
#define RESNET50IMAGENETRECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <opencv2/dnn.hpp>

namespace cv { namespace imgrec {

// https://github.com/BVLC/caffe/wiki/Model-Zoo#resnets-deep-residual-networks-from-msra-at-imagenet-and-coco-2015

class ResNet50ImageNetRecognizer : public CNNImageRecognizer
{
public:
    ResNet50ImageNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold);

    Mat   getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const override;
    Mat   getImageDescription(const Mat &_img) const override;
    void  predict(InputArray src, Ptr<PredictCollector> collector) const override;
};

Ptr<CNNImageRecognizer> createResNet50ImageNetRecognizer(const cv::String &_prototextfilename="ResNet-50-deploy.prototxt", const cv::String &_caffemodelfilename="ResNet-50-model.caffemodel", DistanceType _disttype=DistanceType::Cosine, double _threshold=DBL_MAX);

}}

#endif // RESNET50IMAGENETRECOGNIZER_H
