// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef __OPENCV_CNNIMGREC_HPP__
#define __OPENCV_CNNIMGREC_HPP__

#include <opencv2/dnn.hpp>

#include "imagerec_basic.hpp"
#include "imagerecognizer.hpp"

namespace cv { namespace oirt {

class CNNImageRecognizer : public ImageRecognizer
{
public:
    CNNImageRecognizer(Size _inputsize, CropMethod _cropinput, ColorOrder _colororder, DistanceType _disttype, double _threshold);
    virtual ~CNNImageRecognizer() override;

    void    train(InputArrayOfArrays src, InputArray labels, bool _visualize=true) override;
    void    update(InputArrayOfArrays src, InputArray labels, bool _visualize=true, int *_error=nullptr) override;
    void    addKnownDescription(const cv::Mat &_dscrmat, int _label);
    int     remove(InputArray labels) override;
    void    load(const FileStorage &fs) override;
    void    save(FileStorage &fs) const override;
    void    clear() override;
    bool    empty() const override;
    bool    emptyWhitelist() const;
    int     nextfreeLabel() const;
    /**
     * @brief Set whitelist of labels allowed for identification
     * @param _vlabelinfo - vector of labels identifiers
     */
    void setWhitelist(const std::vector<cv::String> &_vlabelinfo);
    /**
     * @brief Add all known labels to the whitelist
     */
    void dropWhitelist();

    /**
     * @brief Call to get number of knowing templates for the particualr label
     * @param _label
     * @return self explained
     */
    int labelTemplates(int _label) const;

    bool isLabelWhitelisted(int _label) const;

    virtual Mat  getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error=nullptr) const = 0;

protected:
    void __train(InputArrayOfArrays _src, InputArray _labels, bool _preserveData, bool _visualize, int *_error=nullptr);

    int                                 inputChannels;
    std::vector<int>                    v_labels;
    std::vector<Mat>                    v_descriptions;
    std::vector<uchar>                  v_whitelist;
};

}}

#endif //__OPENCV_CNNIMGREC_HPP__
