#include "cnnimageclassifier.h"

namespace cv { namespace oirt {

CNNImageClassifier::CNNImageClassifier(Size _inputsize, int _inputchannels, CropMethod _cropinput) :
    ImageClassifier(_inputsize,_inputchannels,_cropinput)
{
}

CNNImageClassifier::~CNNImageClassifier()
{
}

}}
