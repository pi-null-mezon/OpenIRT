#include "cnnimageclassifier.h"

namespace cv { namespace oirt {

CNNImageClassifier::CNNImageClassifier(Size _inputsize, ColorOrder _colororder, CropMethod _cropinput) :
    ImageClassifier(_inputsize,_colororder,_cropinput)
{
}

CNNImageClassifier::~CNNImageClassifier()
{
}

}}
