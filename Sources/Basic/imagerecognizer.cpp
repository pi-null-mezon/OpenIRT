/*
 * Copyright (c) 2011,2012. Philipp Wagner <bytefish[at]gmx[dot]de>.
 * Released to public domain under terms of the BSD Simplified license.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the organization nor the names of its contributors
 *     may be used to endorse or promote products derived from this software
 *     without specific prior written permission.
 *
 *   See <http://www.opensource.org/licenses/bsd-license>
 */

#include "imagerecognizer.hpp"

namespace cv { namespace oirt {

ImageRecognizer::ImageRecognizer(Size _inputsize, CropMethod _cropinput, ColorOrder _colororder, DistanceType _distancetype, double _threshold) :
    distanceType(_distancetype),    
    threshold(_threshold),
    inputSize(_inputsize),
    cropInput(_cropinput),
    colororder(_colororder)
{
}

ImageRecognizer::~ImageRecognizer()
{
}

std::vector<int> ImageRecognizer::getLabelsByString(const String &str) const
{
    std::vector<int> labels;
    for (std::map<int, String>::const_iterator it = labelsInfo.begin(); it != labelsInfo.end(); it++) {
        if((it->second).compare(str) == 0) // the contents of both strings are equal
            labels.push_back(it->first);
    }
    return labels;
}

String ImageRecognizer::getErrorInfo(int _error) const
{
    std::map<int, String>::const_iterator iter(errorsInfo.find(_error));
    return iter != labelsInfo.end() ? iter->second : "Not defined";
}

double ImageRecognizer::getThreshold() const
{
    return threshold;
}

void ImageRecognizer::setThreshold(double val)
{
    threshold = val;
}

DistanceType ImageRecognizer::getDistanceType() const
{
    return distanceType;
}

Size ImageRecognizer::getInputSize() const
{
    return inputSize;
}

CropMethod ImageRecognizer::getCropInput() const
{
    return cropInput;
}

std::map<int,String> ImageRecognizer::getLabelsInfo() const
{
    return labelsInfo;
}

ColorOrder ImageRecognizer::getColorOrder() const
{
    return colororder;
}

String ImageRecognizer::getLabelInfo(int label) const
{
    std::map<int, String>::const_iterator iter(labelsInfo.find(label));
    return iter != labelsInfo.end() ? iter->second : "";
}

void ImageRecognizer::setLabelInfo(int label, const String &strInfo)
{
    labelsInfo[label] = strInfo;
}

void ImageRecognizer::load(const String &filename)
{
    FileStorage fs(filename, FileStorage::READ);
    if(!fs.isOpened()) {
        std::cerr << "File " << filename << " can't be opened for reading!\n";
        return;
    }
    this->load(fs);
    fs.release();
}

void ImageRecognizer::save(const String &filename) const
{
    FileStorage fs(filename, FileStorage::WRITE);
    if (!fs.isOpened()) {
        std::cerr << "File " << filename << " can't be opened for writing!\n";
        return;
    }
    this->save(fs);
    fs.release();
}

void ImageRecognizer::predict(InputArray src, int &label, double &distance, int *_error) const {
    Ptr<StandardCollector> collector = StandardCollector::create();
    predict(src, collector,_error);
    label = collector->getMinLabel();
    distance = collector->getMinDist();
}

std::vector<std::pair<int, double>> ImageRecognizer::recognize(InputArray src, bool unique, int *_error) const {
    Ptr<StandardCollector> collector = StandardCollector::create();
    predict(src, collector,_error);
    return collector->getResults(true,unique);
}

double ImageRecognizer::compare(InputArray esrc, InputArray vsrc, int *_error) const
{
    cv::Mat _edscr = getImageDescription(esrc.getMat(),_error);
    cv::Mat _vdscr = getImageDescription(vsrc.getMat(),_error);
    double distance = DBL_MAX;
    switch(getDistanceType()) {
        case DistanceType::Euclidean:
            distance = euclideanDistance(_edscr, _vdscr);
            break;
        case DistanceType::Cosine:
            distance =  cosineDistance(_edscr, _vdscr);
            break;
    }
    return distance;
}

}}

