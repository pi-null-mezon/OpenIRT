/*
By downloading, copying, installing or using the software you agree to this
license. If you do not agree to this license, do not download, install,
copy or use the software.

                          License Agreement
               For Open Source Computer Vision Library
                       (3-clause BSD License)

Copyright (C) 2013, OpenCV Foundation, all rights reserved.
Third party copyrights are property of their respective owners.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.

  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

  * Neither the names of the copyright holders nor the names of the contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

This software is provided by the copyright holders and contributors "as is" and
any express or implied warranties, including, but not limited to, the implied
warranties of merchantability and fitness for a particular purpose are
disclaimed. In no event shall copyright holders or contributors be liable for
any direct, indirect, incidental, special, exemplary, or consequential damages
(including, but not limited to, procurement of substitute goods or services;
loss of use, data, or profits; or business interruption) however caused
and on any theory of liability, whether in contract, strict liability,
or tort (including negligence or otherwise) arising in any way out of
the use of this software, even if advised of the possibility of such damage.
*/

#ifndef __OPENCV_IMAGERECOGNIZER_HPP__
#define __OPENCV_IMAGERECOGNIZER_HPP__

#include <opencv2/core.hpp>
#include "predict_collector.hpp"
#include <map>

namespace cv { namespace imgrec {

enum DistanceType {Euclidean, Cosine};
/**
 * @brief The CropMethod enum defines how image will be preprocessed at
 */
enum CropMethod {NoCrop, Inside, Outside, OutsideJitter};

class ImageRecognizer : public Algorithm
{
public:
    ImageRecognizer(Size _inputsize, int _inputchannels, CropMethod _cropinput, DistanceType _distancetype, double _threshold);

    virtual void train(InputArrayOfArrays src, InputArray labels, bool _visualize) = 0;
    virtual void update(InputArrayOfArrays src, InputArray labels, bool _visualize) = 0;
    virtual int remove(InputArray labels) = 0;

    /**
     * @brief predict label for the input src image
     * @param src - input image
     * @return predicted label
     */
    int predict(InputArray src) const;

    /**
     * @brief predict label for the input src image
     * @param src - input image
     * @param label - predicted label
     * @param distance - computed distance to predicted label
     */
    void predict(InputArray src, int &label, double &distance) const;

    /**
     * @brief get predictions for all unique labels
     * @param src - input images
     * @param unique - should result contains only single one (best in terms of distance) prediction for each label
     * @return pairs of label and distance
     * @note let's consider two particular cases:
     *
     * 'uniquelabels == true' - if recognizer stores N templates for the label L, the result will contain only one single pair for the label L
     *       in this case three steps for the predictions preparation are used:
     *       first all predictions for the particular label will be sorted in the ascending distance order,
     *       second only one prediction for the particular label will be returned (the one with the minimum distance),
     *       thirdly all unique labels will be once again sorted in the ascending distance order
     *
     * 'uniquelabels == false' - if recognizer stores N templates for the label L, the result will contain N pairs for the label L
     *       in this case all predictions are simply sorted in ascending distance order
     */
    std::vector<std::pair<int,double>> recognize(InputArray src, bool unique=true) const;

    virtual void predict(InputArray src, Ptr<PredictCollector> collector) const = 0;

    virtual Mat getImageDescription(const Mat &_img) const = 0;

    virtual void save(const String& filename) const;

    virtual void load(const String& filename);

    virtual void save(FileStorage& fs) const = 0;

    virtual void load(const FileStorage& fs) = 0;

    virtual void setLabelInfo(int label, const String& strInfo);

    virtual String getLabelInfo(int label) const;

    virtual std::vector<int> getLabelsByString(const String& str) const;

    virtual double getThreshold() const;

    virtual void setThreshold(double val);

    virtual DistanceType getDistanceType() const;

    virtual void setDistanceType(DistanceType _type);

    virtual Size getInputSize() const;

    virtual void setInputSize(Size _size);

    virtual int getInputChannels() const;

    virtual void setInputChannels(int _val);

    virtual CropMethod getCropInput() const;

    virtual void setCropInput(CropMethod value);

    std::map<int,String> getLabelsInfo() const;

protected:
    // Stored pairs "label id - string info"
    std::map<int, String> _labelsInfo;

    DistanceType    distanceType;
    double          threshold;
    Size            inputSize;
    int             inputChannels;
    CropMethod      cropInput;
};

}}

#endif
