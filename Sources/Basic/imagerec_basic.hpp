#ifndef __OPENCV_IMGREC_BASIC_HPP
#define __OPENCV_IMGREC_BASIC_HPP

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <iostream>

namespace cv { namespace oirt {

/**
 * @brief The DistanceType enum
 */
enum DistanceType {Euclidean, Cosine};
/**
 * @brief The CropMethod enum defines how image will be preprocessed
 * @note  makes sense only if target size.width !=0 and target size.width != 0
 */
enum CropMethod {NoCrop, Inside, Outside};
/**
 * @brief The ColorOrder enum defines how image will be preprocessed
 */
enum ColorOrder {BGR, RGB, Gray};

// Reads a sequence from a FileNode::SEQ with type _Tp into a result vector.
template<typename _Tp>
inline void readFileNodeList(const FileNode& fn, std::vector<_Tp>& result) {
    if (fn.type() == FileNode::SEQ) {
        result.reserve(fn.size());
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

/**
 * @brief Apply crop inside original image
 * @param input - input image
 * @param size - target size of the output image
 * @return Crop from the input image with dimensions as defined by size
 */
inline cv::Mat cropInsideFromCenterAndResize(const cv::Mat &input, cv::Size size)
{
    cv::Rect2f roiRect(0,0,0,0);
    if((float)input.cols/input.rows > (float)size.width/size.height) {
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
        int interpolationMethod = CV_INTER_AREA;
        if(size.area() > roiRect.area())
            interpolationMethod = CV_INTER_CUBIC;
        cv::resize(croppedImg, output, size, 0, 0, interpolationMethod);
    }
    return output;
}

inline cv::Mat cropOutsideFromCenterAndResize(const cv::Mat &input, cv::Size size)
{
    cv::Size2f _bsize;
    if((float)input.cols/input.rows > (float)size.width/size.height) {
        _bsize.width = (float)input.cols;
        _bsize.height = input.cols * (float)size.height/size.width;
    } else {
        _bsize.height = (float)input.rows;
        _bsize.width = input.rows * (float)size.width/size.height;
    }
    cv::Mat output(_bsize,input.type(),cv::Scalar(104,117,123));
    input.copyTo(cv::Mat(output,cv::Rect2f(((float)_bsize.width - (float)input.cols)/2.0f,
                                           ((float)_bsize.height - (float)input.rows)/2.0f,
                                           (float)input.cols,(float)input.rows)));

    int interpolationMethod = CV_INTER_AREA;
    if(size.area() > _bsize.area())
        interpolationMethod = CV_INTER_CUBIC;
    cv::resize(output, output, size, 0, 0, interpolationMethod);
    return output;
}

/**
 * @brief Apply random affine transformation to image
 * @param _inmat - input image
 * @param _cvrng - random number generator
 * @param _targetsize - target output image size
 * @param _maxscale - scale deviation
 * @param _maxshift - translation deviation
 * @param _maxangle - angle deviation in degrees
 * @param _bordertype - opencv border type
 * @param _alwaysshrink - if scale should be always less or equal to 1
 * @return transformed image
 */
inline cv::Mat jitterimage(const cv::Mat &_inmat, cv::RNG &_cvrng, const cv::Size &_targetsize=cv::Size(0,0), double _maxscale=0.05, double _maxshift=0.03, double _maxangle=7, int _bordertype=cv::BORDER_REFLECT101, bool _alwaysshrink=true)
{
    cv::Mat _outmat;
    const cv::Size _insize(_inmat.cols,_inmat.rows);
    double _scale = 1.;
    if(_targetsize.area() > 0)
        _scale = std::min((double)_targetsize.width/_insize.width, (double)_targetsize.height/_insize.height);
    cv::Mat _matrix = cv::getRotationMatrix2D(cv::Point2f(_inmat.cols/2.f,_inmat.rows/2.f),
                                              _maxangle * (_cvrng.uniform(0.,2.) - 1.),
                                              _alwaysshrink ? _scale * (1. - _maxscale*_cvrng.uniform(0.,1.0)) : _scale * (1. + _maxscale*(_cvrng.uniform(0.,2.) - 1.)));
    if((_targetsize.width > 0) && (_targetsize.height > 0)) {
        _matrix.at<double>(0,2) += -(_insize.width - _targetsize.width) / 2.;
        _matrix.at<double>(1,2) += -(_insize.height - _targetsize.height) / 2.;
    }
    _matrix.at<double>(0,2) += (_insize.width * _maxshift * _scale * (_cvrng.uniform(0.,2.) - 1.));
    _matrix.at<double>(1,2) += (_insize.height * _maxshift * _scale * (_cvrng.uniform(0.,2.) - 1.));
    cv::warpAffine(_inmat,_outmat,_matrix,
                   _targetsize,
                   _insize.area() > _targetsize.area() ? CV_INTER_AREA : CV_INTER_CUBIC,
                   _bordertype,cv::mean(_inmat));
    return _outmat;
}

inline cv::Mat preprocessImageForCNN(const Mat &_inmat, Size _targetsize, ColorOrder _targetcolororder, CropMethod _crop)
{
    cv::Mat _outmat;
    // First we need to make resize if needed
    if((_targetsize.width != 0) && (_targetsize.height != 0)) {
        if((_inmat.cols != _targetsize.width) || (_inmat.rows != _targetsize.height)) {
            switch(_crop) {

                case CropMethod::NoCrop: {
                    int _im = CV_INTER_AREA;
                    if(_targetsize.area() > (_inmat.rows*_inmat.cols))
                        _im = CV_INTER_CUBIC;
                    cv::resize(_inmat, _outmat, _targetsize, 0, 0, _im);
                } break;

                case CropMethod::Inside:
                    _outmat = cropInsideFromCenterAndResize(_inmat, _targetsize);
                    break;

                case CropMethod::Outside:
                    _outmat = cropOutsideFromCenterAndResize(_inmat, _targetsize);
                    break;
            }
        } else {
            _outmat = _inmat;
        }
    } else {
        _outmat = _inmat;
    }
    // Now we should optionally convert color scheme
    int _convcode = -1;
    switch(_targetcolororder) {
        case ColorOrder::RGB:
            switch(_outmat.channels()) {
                case 4:
                    _convcode = CV_BGRA2RGB;
                    break;
                case 3:
                    _convcode = CV_BGR2RGB;
                    break;
                case 1:
                    _convcode = CV_GRAY2RGB;
                    break;
            }
            break;

        case ColorOrder::BGR:
            switch(_outmat.channels()) {
                case 4:
                    _convcode = CV_BGRA2BGR;
                    break;
                case 1:
                    _convcode = CV_GRAY2BGR;
                    break;
            }
            break;

        case ColorOrder::Gray:
            switch(_outmat.channels()) {
                case 4:
                    _convcode = CV_BGRA2GRAY;
                    break;
                case 3:
                    _convcode = CV_BGR2GRAY;
                    break;
            }
            break;
    }
    if(_convcode != -1) {
        cv::cvtColor(_outmat,_outmat,_convcode);
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

