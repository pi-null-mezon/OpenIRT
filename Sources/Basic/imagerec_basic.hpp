#ifndef __OPENCV_IMGREC_BASIC_HPP
#define __OPENCV_IMGREC_BASIC_HPP

#include <set>
#include <limits>
#include <iostream>

#include "imagerecognizer.hpp"
#include "precompiled.hpp"

namespace cv { namespace imgrec {

// Reads a sequence from a FileNode::SEQ with type _Tp into a result vector.
template<typename _Tp>
inline void readFileNodeList(const FileNode& fn, std::vector<_Tp>& result) {
    if (fn.type() == FileNode::SEQ) {
        for (FileNodeIterator it = fn.begin(); it != fn.end();) {
            _Tp item;
            it >> item;
            result.push_back(item);
        }
    }
}

// Writes the a list of given items to a cv::FileStorage.
template<typename _Tp>
inline void writeFileNodeList(FileStorage& fs, const String& name,
                              const std::vector<_Tp>& items) {
    // typedefs
    typedef typename std::vector<_Tp>::const_iterator constVecIterator;
    // write the elements in item to fs
    fs << name << "[";
    for (constVecIterator it = items.begin(); it != items.end(); ++it) {
        fs << *it;
    }
    fs << "]";
}

// Utility structure to load/save imgrec label info (a pair of int and string) via FileStorage
struct LabelInfo
{
    LabelInfo():label(-1), value("") {}
    LabelInfo(int _label, const String &_value): label(_label), value(_value) {}
    int label;
    String value;
    void write(cv::FileStorage& fs) const
    {
        fs << "{" << "label" << label << "value" << value << "}";
    }
    void read(const cv::FileNode& node)
    {
        label = (int)node["label"];
        value = (String)node["value"];
    }
    std::ostream& operator<<(std::ostream& out)
    {
        out << "{ label = " << label << ", " << "value = " << value.c_str() << "}";
        return out;
    }
};

inline void write(cv::FileStorage& fs, const String&, const LabelInfo& x)
{
    x.write(fs);
}

inline void read(const cv::FileNode& node, LabelInfo& x, const LabelInfo& default_value = LabelInfo())
{
    if(node.empty())
        x = default_value;
    else
        x.read(node);
}

inline Mat cropFromCenterAndResize(const cv::Mat &input, cv::Size size)
{
    cv::Rect2f roiRect(0,0,0,0);
    if( (float)input.cols/input.rows > (float)size.width/size.height) {
        roiRect.height = (float)input.rows;
        roiRect.width = input.rows * (float)size.width/size.height;
        roiRect.x = (input.cols - roiRect.width)/2.0f;
    } else {
        roiRect.width = (float)input.cols;
        roiRect.height = input.cols * (float)size.height/size.width;
        roiRect.y = (input.rows - roiRect.height)/2.0f;
    }
    roiRect &= cv::Rect2f(0, 0, (float)input.cols, (float)input.rows);
    cv::Mat output;
    if(roiRect.area() > 0)  {
        cv::Mat croppedImg(input, roiRect);
        int interpolationMethod = 0;
        if(size.area() > roiRect.area())
            interpolationMethod = CV_INTER_CUBIC;
        else
            interpolationMethod = CV_INTER_AREA;
        cv::resize(croppedImg, output, size, 0, 0, interpolationMethod);
    }
    return output;
}

inline Mat preprocessImageForCNN(const Mat &_inmat, Size _targetsize, int _targetchannels, bool _crop)
{
    cv::Mat _outmat;
	if((_inmat.channels() == 4) && (_targetchannels == 3)) {
        cv::cvtColor(_inmat,_outmat,CV_BGRA2BGR);
	} else if((_inmat.channels() == 4) && (_targetchannels == 1)) {
		cv::cvtColor(_inmat,_outmat,CV_BGRA2GRAY);
	} else if((_inmat.channels() == 3) && (_targetchannels == 3)) {
        //cv::cvtColor(_inmat,_outmat,CV_BGR2RGB);
        _outmat = _inmat;
	} else if((_inmat.channels() == 3) && (_targetchannels == 1)) {
		cv::cvtColor(_inmat,_outmat,CV_BGR2GRAY);
    } else if((_inmat.channels() == 1) && (_targetchannels == 3)) {
        cv::cvtColor(_inmat,_outmat,CV_GRAY2BGR);
    } else {
        _outmat = _inmat;
    }    
    if(_targetsize.width != 0 && _targetsize.height != 0) {
        if((_outmat.cols != _targetsize.width) || (_outmat.rows != _targetsize.height)) {
            if(_crop) {
                _outmat = cropFromCenterAndResize(_outmat, _targetsize);
            } else {
                int _im = CV_INTER_AREA;
                if(_targetsize.area() > (_outmat.rows*_outmat.cols))
                    _im = CV_INTER_CUBIC;
                cv::resize(_outmat, _outmat, _targetsize, 0, 0, _im);
            }
        }
    }
    return _outmat;
}

inline double euclideanDistance(const Mat &_dmat1, const Mat &_dmat2)
{
    // it is approximately two times faster than the cv::compareHist
    return cv::norm(_dmat1, _dmat2, NORM_L2);
}

inline double cosineDistance(const Mat &_dmat1, const Mat &_dmat2)
{
    double _n1 = cv::norm(_dmat1, NORM_L2);
    double _n2 = cv::norm(_dmat2, NORM_L2);
    return (1.0 - _dmat1.dot(_dmat2)/(_n1*_n2));
}

}}

#endif // __OPENCV_IMGREC_BASIC_HPP

