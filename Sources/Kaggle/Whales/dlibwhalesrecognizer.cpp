#include "dlibwhalesrecognizer.h"

#include <opencv2/highgui.hpp>

namespace cv { namespace imgrec {

DlibWhalesRecognizer::DlibWhalesRecognizer(const String &_descriptormodelfile, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(500,200),3,false,_disttype,_threshold)
{
    try {
        dlib::deserialize(_descriptormodelfile.c_str()) >> net;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

Mat DlibWhalesRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const
{
    cv::String _str = _blobname; // to suppress 'unused variable' compiler warning
    // Prepare image
    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getInputChannels(), getCropInput());

    //-----------------------------
    /*cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getInputChannels(), getCropInput());
    _preprocessedmat.convertTo(_preprocessedmat,CV_32F);
    cv::Mat _vchannelmean, _vchannelstdev;
    cv::meanStdDev(_preprocessedmat,_vchannelmean,_vchannelstdev);
    cv::Mat _nmat = (_preprocessedmat - _vchannelmean.at<const double>(0)) / _vchannelstdev.at<const double>(0);

    float *_p = _nmat.ptr<float>(0);
    dlib::matrix<float> _dlibimg(_nmat.rows,_nmat.cols);
    for(long i = 0; i < _nmat.rows*_nmat.cols; ++i)
        _dlibimg(i) = _p[i];

    dlib::matrix<float,0,1> _description = net(_dlibimg);*/
    //-----------------------------
    dlib::cv_image<dlib::rgb_pixel> _dlibcvimg(_preprocessedmat);
    dlib::matrix<dlib::rgb_pixel> _dlibmatrix;
    dlib::assign_image(_dlibmatrix,_dlibcvimg);
    // Get description
    dlib::matrix<float,0,1> _description = net(_dlibmatrix);
    // Perform forward propagation
    return dlib::toMat(_description).reshape(1,1).clone();
}

Mat DlibWhalesRecognizer::getImageDescription(const Mat &_img) const
{
    return getImageDescriptionByLayerName(_img,cv::String());
}

void DlibWhalesRecognizer::predict(InputArray src, Ptr<PredictCollector> collector) const
{
    cv::Mat _description = getImageDescription(src.getMat());
    collector->init(v_labels.size());
    for (size_t sampleIdx = 0; sampleIdx < v_labels.size(); sampleIdx++) {
        double confidence = DBL_MAX;
        switch(getDistanceType()) {
            case DistanceType::Euclidean:
                confidence = euclideanDistance(v_descriptions[sampleIdx], _description);
                break;
            case DistanceType::Cosine:
                confidence =  cosineDistance(v_descriptions[sampleIdx], _description);
                break;
        }
        if( !collector->collect(v_labels[sampleIdx], confidence) ) {
            return;
        }
    }
}

Ptr<CNNImageRecognizer> createDlibWhalesRecognizer(const String &_descriptormodelfile, DistanceType _disttype, double _threshold)
{
    return makePtr<DlibWhalesRecognizer>(_descriptormodelfile,_disttype,_threshold);
}

}
}
