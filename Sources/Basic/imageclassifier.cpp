#include "imageclassifier.h"

namespace cv { namespace oirt {

ImageClassifier::ImageClassifier(Size _inputsize, ColorOrder _colororder, CropMethod _cropinput) :
    inputSize(_inputsize),
    cropInput(_cropinput),
    colororder(_colororder)
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

CropMethod ImageClassifier::getCropInput() const
{
    return cropInput;
}

void ImageClassifier::setCropInput(const CropMethod &value)
{
    cropInput = value;
}

ColorOrder ImageClassifier::getColorOrder() const
{
    return colororder;
}

}}
