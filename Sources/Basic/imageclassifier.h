#ifndef IMAGECLASSIFIER_H
#define IMAGECLASSIFIER_H

#include "imagerec_basic.hpp"

#include <map>

namespace cv { namespace oirt {

/**
 * This class and his derivatives do not wait any training
 * (contrary to the ImageRecognizer). All outputs are fixed and
 * determined only by it's resources. It was made as interface to
 * pretrained models with known labels
 */
class ImageClassifier : public Algorithm
{
public:
    ImageClassifier(Size _inputsize, int _inputchannels, CropMethod _cropinput);
    virtual ~ImageClassifier();

    /**
     * @brief predict label for src input
     * @param src - input image
     * @param label - predicted label
     * @param conf - confidence of the prediction
     */
    virtual void predict(InputArray src, int &label, double &conf) const = 0;

    virtual String getLabelInfo(int label) const;
    virtual void setLabelInfo(int label, const String& strInfo);

    Size getInputSize() const;
    void setInputSize(const Size &value);

    int getInputChannels() const;
    void setInputChannels(int value);

    CropMethod getCropInput() const;
    void setCropInput(const CropMethod &value);

protected:
    std::map<int, String> labelsInfo;
    Size            inputSize;
    int             inputChannels;
    CropMethod      cropInput;
};

}}

#endif // IMAGECLASSIFIER_H
