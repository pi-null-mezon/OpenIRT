#include "squeezenetimagenetrecognizer.h"

namespace cv { namespace oirt {

SqueezeNetImageNetRecognizer::SqueezeNetImageNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(Size(227,227), 3, CropMethod::Inside, _disttype, _threshold)
{
    try {
        net = dnn::readNetFromCaffe(_prototextfilename,_caffemodelfilename);
    } catch (const cv::Exception &err) {
        std::cerr << err.msg << std::endl;
    }
}

Mat SqueezeNetImageNetRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    // Prepare image
    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getInputChannels(), getCropInput());
    // Set image as network input data blob
    cv::Mat inputBlob = dnn::blobFromImage(_preprocessedmat,1,Size(),cv::Scalar(104,117,123)); // this values has been copied from https://github.com/BVLC/caffe/blob/master/models/bvlc_googlenet/train_val.prototxt
    net.setInput(inputBlob, "data");
    // Perform forward propagation
    Mat _dscrmat = net.forward(_blobname);
    // Return description of the face as real cv::Mat vector
    return _dscrmat.reshape(1,1).clone(); // clonning is necessasry here
}

Mat SqueezeNetImageNetRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    return getImageDescriptionByLayerName(_img, "pool10", _error); // Search answers in the bvlc_googlenet.prototxt
}

void SqueezeNetImageNetRecognizer::predict(InputArray src, Ptr<PredictCollector> collector, int *_error) const
{
    cv::Mat _description = getImageDescription(src.getMat(), _error);
    collector->init(v_labels.size());
    for (size_t sampleIdx = 0; sampleIdx < v_labels.size(); sampleIdx++) {
        double confidence = DBL_MAX;
        switch(getDistanceType()) {
            case DistanceType::Euclidean:
                confidence = euclideanDistance(v_descriptions[sampleIdx], _description);
                break;
            case DistanceType::Cosine:
                confidence =  cosineDistance(v_descriptions[sampleIdx], _description);
                break;
        }
        if( !collector->collect(v_labels[sampleIdx], confidence) ) {
            return;
        }
    }
}

void SqueezeNetImageNetRecognizer::setPreferableTarget(int _targetId)
{
    net.setPreferableTarget(_targetId);
}

void cv::oirt::SqueezeNetImageNetRecognizer::setPreferableBackend(int _backendId)
{
    net.setPreferableBackend(_backendId);
}

Ptr<CNNImageRecognizer> createSqueezeNetImageNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold)
{
    return makePtr<SqueezeNetImageNetRecognizer>(_prototextfilename,_caffemodelfilename,_disttype,_threshold);
}

}}
