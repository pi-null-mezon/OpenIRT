#ifndef CNNIMAGECLASSIFIER_H
#define CNNIMAGECLASSIFIER_H

#include "imageclassifier.h"

namespace cv { namespace oirt {

class CNNImageClassifier : public ImageClassifier
{
public:
    CNNImageClassifier(Size _inputsize, ColorOrder _colororder, CropMethod _cropinput);
    virtual ~CNNImageClassifier();
};

}}

#endif // CNNIMAGECLASSIFIER_H
