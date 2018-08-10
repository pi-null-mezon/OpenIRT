#include "imageclassifier.h"

namespace cv { namespace oirt {

ImageClassifier::ImageClassifier(Size _inputsize, int _inputchannels, CropMethod _cropinput) :
    inputSize(_inputsize),
    inputChannels(_inputchannels),
    cropInput(_cropinput)
{
}

ImageClassifier::~ImageClassifier()
{
}

String ImageClassifier::getLabelInfo(int label) const
{
    std::map<int, String>::const_iterator iter(labelsInfo.find(label));
    return iter != labelsInfo.end() ? iter->second : "";
}

void ImageClassifier::setLabelInfo(int label, const String &strInfo)
{
    labelsInfo[label] = strInfo;
}

Size ImageClassifier::getInputSize() const
{
    return inputSize;
}

void ImageClassifier::setInputSize(const Size &value)
{
    inputSize = value;
}

int ImageClassifier::getInputChannels() const
{
    return inputChannels;
}

void ImageClassifier::setInputChannels(int value)
{
    inputChannels = value;
}

CropMethod ImageClassifier::getCropInput() const
{
    return cropInput;
}

void ImageClassifier::setCropInput(const CropMethod &value)
{
    cropInput = value;
}

}}
