// This file is part of OpenCV project.
// It is subject to the license terms in the LICENSE file found in the top-level directory
// of this distribution and at http://opencv.org/license.html.

#ifndef __OPENCV_CNNIMGREC_HPP__
#define __OPENCV_CNNIMGREC_HPP__

#include "imagerec_basic.hpp"
#include <opencv2/highgui.hpp>

namespace cv { namespace imgrec {

class CNNImageRecognizer : public ImageRecognizer
{
public:
    CNNImageRecognizer(DistanceType _distancetype, double _threshold) : ImageRecognizer(_distancetype, _threshold) {}

    virtual Size 	getInputSize() const = 0;
    virtual void 	setInputSize(Size _size) = 0;
    virtual int 	getInputChannels() const = 0;
    virtual void 	setInputChannels(int _val) = 0;    
};

class CNNImageRecognizerBasicImpl : public CNNImageRecognizer
{
public:
    CNNImageRecognizerBasicImpl(Size _inputsize, int _inputchannels, DistanceType _disttype, double _threshold);

    void    train(InputArrayOfArrays src, InputArray labels) override;
    void    update(InputArrayOfArrays src, InputArray labels) override;
    void    load(const FileStorage &fs) override;
    void    save(FileStorage &fs) const override;    
    Size 	getInputSize() const override;
    void 	setInputSize(Size _size) override;
    int 	getInputChannels() const override;
    void 	setInputChannels(int _val) override;    

protected:
    void __train(InputArrayOfArrays _src, InputArray _labels, bool _preserveData);

    Size                                inputSize;
    int                                 inputChannels;
    std::vector<int>                    v_labels;
    std::vector<Mat>                    v_descriptions;
};

}} //namespace cv::imgrec

#endif //__OPENCV_CNNIMGREC_HPP__
