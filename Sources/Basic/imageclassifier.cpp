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

String ImageClassifier::getErrorInfo(int _error)
{
    std::map<int, String>::const_iterator iter(errorsInfo.find(_error));
    return iter != labelsInfo.end() ? iter->second : "Not defined";
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

std::map<int, String> ImageClassifier::getLabelsInfo() const
{
    return labelsInfo;
}

}}
