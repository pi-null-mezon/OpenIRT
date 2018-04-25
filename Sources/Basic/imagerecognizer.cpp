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
#include <iostream>

namespace cv { namespace imgrec {

ImageRecognizer::ImageRecognizer(Size _inputsize, int _inputchannels, bool _cropinput, DistanceType _distancetype, double _threshold) :
    distanceType(_distancetype),
    threshold(_threshold),
    inputSize(_inputsize),
    inputChannels(_inputchannels),
    cropInput(_cropinput)
{

}

std::vector<int> ImageRecognizer::getLabelsByString(const String &str) const
{
  std::vector<int> labels;
  for (std::map<int, String>::const_iterator it = _labelsInfo.begin(); it != _labelsInfo.end(); it++) {
      size_t found = (it->second).find(str);
      if (found != String::npos)
          labels.push_back(it->first);
  }
  return labels;
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

void ImageRecognizer::setDistanceType(DistanceType _type)
{
    distanceType = _type;
}

Size ImageRecognizer::getInputSize() const
{
    return inputSize;
}

void ImageRecognizer::setInputSize(Size _size)
{
    inputSize = _size;
}

int ImageRecognizer::getInputChannels() const
{
    return inputChannels;
}

void ImageRecognizer::setInputChannels(int _val)
{
    inputChannels = _val;
}

bool ImageRecognizer::getCropInput() const
{
    return cropInput;
}

void ImageRecognizer::setCropInput(bool value)
{
    cropInput = value;
}

String ImageRecognizer::getLabelInfo(int label) const
{
    std::map<int, String>::const_iterator iter(_labelsInfo.find(label));
    return iter != _labelsInfo.end() ? iter->second : "";
}

void ImageRecognizer::setLabelInfo(int label, const String &strInfo)
{
    _labelsInfo[label] = strInfo;
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

int ImageRecognizer::predict(InputArray src) const {
    int _label;
    double _dist;
    predict(src, _label, _dist);
    return _label;
}

void ImageRecognizer::predict(InputArray src, int &label, double &distance) const {
    Ptr<StandardCollector> collector = StandardCollector::create(getThreshold());
    predict(src, collector);
    label = collector->getMinLabel();
    distance = collector->getMinDist();
}

std::vector<std::pair<int, double> > ImageRecognizer::recognize(InputArray src, bool unique) const {
    Ptr<StandardCollector> collector = StandardCollector::create(getThreshold());
    predict(src, collector);
    return collector->getResults(true,unique);
}


}
}

