#include "resnet50imagenetrecognizer.h"

namespace cv { namespace imgrec {

ResNet50ImageNetRecognizer::ResNet50ImageNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(Size(224,224), 3, _disttype, _threshold)
{
    Ptr<dnn::Importer> importer;
    try {
        importer = dnn::createCaffeImporter(_prototextfilename, _caffemodelfilename);
    } catch (const cv::Exception &err) {
        std::cerr << err.msg << std::endl;
    }
    if(importer) {
        importer->populateNet(net);
    }
    importer.release(); // free the memory
}

Mat ResNet50ImageNetRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const
{
    // Prepare image
    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getInputChannels());
    // Set image as network input data blob
    cv::Mat inputBlob = dnn::blobFromImage(_preprocessedmat);
    net.setInput(inputBlob, "data");
    // Perform forward propagation
    Mat _dscrmat = net.forward(_blobname);
    // Return description of the face as real cv::Mat vector
    return _dscrmat.reshape(1,1).clone(); // clonning is necessasry here
}

Mat ResNet50ImageNetRecognizer::getImageDescription(const Mat &_img) const
{
    return getImageDescriptionByLayerName(_img, "fc1000"); // Search answers in the ResNet-50-deploy.prototxt
}

void ResNet50ImageNetRecognizer::predict(InputArray src, Ptr<PredictCollector> collector) const
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

Ptr<CNNImageRecognizer> createResNet50ImageNetRecognizer(const String &_prototextfilename, const String &_caffemodelfilename, DistanceType _disttype, double _threshold)
{
    return makePtr<ResNet50ImageNetRecognizer>(_prototextfilename,_caffemodelfilename,_disttype,_threshold);
}

}}
