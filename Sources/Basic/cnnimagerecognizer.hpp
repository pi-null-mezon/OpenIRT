// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef __OPENCV_CNNIMGREC_HPP__
#define __OPENCV_CNNIMGREC_HPP__

#include <opencv2/dnn.hpp>

#include "imagerec_basic.hpp"

namespace cv { namespace imgrec {

class CNNImageRecognizer : public ImageRecognizer
{
public:
    CNNImageRecognizer(Size _inputsize, int _inputchannels, bool _cropinput, DistanceType _disttype, double _threshold);

    void    train(InputArrayOfArrays src, InputArray labels, bool _visualize=true) override;
    void    update(InputArrayOfArrays src, InputArray labels, bool _visualize=true) override;
    void    remove(InputArray labels) override;
    void    load(const FileStorage &fs) override;
    void    save(FileStorage &fs) const override;
    bool    empty() const override;
    int     nextfreeLabel() const;

    virtual Mat  getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const = 0;

protected:
    void __train(InputArrayOfArrays _src, InputArray _labels, bool _preserveData, bool _visualize);

    Size                                inputSize;
    int                                 inputChannels;
    std::vector<int>                    v_labels;
    std::vector<Mat>                    v_descriptions;    
};

}} //namespace cv::imgrec

#endif //__OPENCV_CNNIMGREC_HPP__