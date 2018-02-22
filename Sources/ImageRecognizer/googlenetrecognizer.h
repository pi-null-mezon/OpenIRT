#ifndef GOOGLENETRECOGNIZER_H
#define GOOGLENETRECOGNIZER_H

#include "cnnimagerecognizer.hpp"

#include <opencv2/dnn.hpp>

namespace cv { namespace imgrec {

// https://github.com/BVLC/caffe/tree/master/models/bvlc_googlenet

class GoogleNetRecognizer : public CNNImageRecognizer
{
public:
    GoogleNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold);

    Mat   getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const override;
    Mat   getImageDescription(const Mat &_img) const override;
    void  predict(InputArray src, Ptr<PredictCollector> collector) const override;

protected:
    mutable dnn::Net net;
};

Ptr<CNNImageRecognizer> createGoogleNetRecognizer(const cv::String &_prototextfilename="bvlc_googlenet.prototxt", const cv::String &_caffemodelfilename="bvlc_googlenet.caffemodel", DistanceType _disttype=DistanceType::Cosine, double _threshold=DBL_MAX);

}}

#endif // GOOGLELENETRECOGNIZER_H
