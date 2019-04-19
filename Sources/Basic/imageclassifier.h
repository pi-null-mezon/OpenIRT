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
    ImageClassifier(Size _inputsize, ColorOrder _colororder, CropMethod _cropinput);
    virtual ~ImageClassifier();

    /**
     * @brief predict label for src input
     * @param src - input image
     * @param label - predicted label
     * @param conf - confidence of the prediction
     */
    virtual void predict(InputArray src, int &label, double &conf, int *_error=nullptr) const = 0;
    /**
     * @brief predict label for src input
     * @param src - input image
     * @param conf - vector of per-label confidences
     */
    virtual void predict(InputArray src, std::vector<double> &conf, int *_error=nullptr) const = 0;

    virtual String getLabelInfo(int label) const;
    virtual void setLabelInfo(int label, const String& strInfo);

    virtual String getErrorInfo(int _error);

    Size getInputSize() const;

    CropMethod getCropInput() const;
    void setCropInput(const CropMethod &value);

    ColorOrder getColorOrder() const;

protected:
    std::map<int, String> labelsInfo;
    // errorsInfo should be defined in the descendants, zero error will be used as "no error" code
    std::map<int, String> errorsInfo;
    Size            inputSize;
    CropMethod      cropInput;
    ColorOrder      colororder;
};

}}

#endif // IMAGECLASSIFIER_H
