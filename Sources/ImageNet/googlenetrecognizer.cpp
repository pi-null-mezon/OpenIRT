#include "googlenetrecognizer.h"

namespace cv { namespace imgrec {

GoogleNetRecognizer::GoogleNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(Size(224,224), 3, CropMethod::Inside, _disttype, _threshold)
{
    try {
        net = dnn::readNetFromCaffe(_prototextfilename,_caffemodelfilename);
    } catch (const cv::Exception &err) {
        std::cerr << err.msg << std::endl;
    }
}

Mat GoogleNetRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const
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

Mat GoogleNetRecognizer::getImageDescription(const Mat &_img) const
{
    return getImageDescriptionByLayerName(_img, "loss3/classifier"); // Search answers in the bvlc_googlenet.prototxt
}

void GoogleNetRecognizer::predict(InputArray src, Ptr<PredictCollector> collector) const
{
    cv::Mat _description = getImageDescription(src.getMat());
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

void GoogleNetRecognizer::setPreferableTarget(int _targetId)
{
    net.setPreferableTarget(_targetId);
}

void GoogleNetRecognizer::setPreferableBackend(int _backendId)
{
    net.setPreferableBackend(_backendId);
}

Ptr<CNNImageRecognizer> createGoogleNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold)
{
    return makePtr<GoogleNetRecognizer>(_prototextfilename,_caffemodelfilename,_disttype,_threshold);
}

}}
