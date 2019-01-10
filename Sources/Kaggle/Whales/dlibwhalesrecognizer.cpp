#include "dlibwhalesrecognizer.h"

namespace cv { namespace oirt {

dlib::matrix<float> cvmat2dlibmatrix(const cv::Mat &_cvmat)
{
    assert(_cvmat.channels() == 1);
    assert(_cvmat.depth() == CV_32F);
    cv::Mat _mat = _cvmat;
    if(_cvmat.isContinuous() == false)
        _mat = _cvmat.clone();
    float *_p = _mat.ptr<float>(0);
    dlib::matrix<float> _img(_cvmat.rows,_cvmat.cols);
    for(long i = 0; i < _cvmat.cols*_cvmat.rows; ++i)
        _img(i) = _p[i];
    return _img;
}

DlibWhalesRecognizer::DlibWhalesRecognizer(const String &_descriptormodelfile, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(512,192),CropMethod::NoCrop,ColorOrder::Gray,_disttype,_threshold)
{
    try {
        dlib::deserialize(_descriptormodelfile.c_str()) >> net;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

Mat DlibWhalesRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    cv::String _str = _blobname; // to suppress 'unused variable' compiler warning

    // Prepare image
    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput(),cvrng);

    _preprocessedmat.convertTo(_preprocessedmat,CV_32F);
    cv::Mat _vchannelmean, _vchannelstdev;
    cv::meanStdDev(_preprocessedmat,_vchannelmean,_vchannelstdev);
    _preprocessedmat = (_preprocessedmat - _vchannelmean.at<const double>(0)) / (3.0*_vchannelstdev.at<const double>(0));

    // Get description
    dlib::matrix<float,0,1> _description = net(cvmat2dlibmatrix(_preprocessedmat));


    /*std::vector<dlib::matrix<float>> _crops;
    _crops.resize(33);
    for(size_t i = 0; i < _crops.size(); ++i) {
        cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput(),cvrng);
        _preprocessedmat.convertTo(_preprocessedmat,CV_32F);
        cv::Mat _vchannelmean, _vchannelstdev;
        cv::meanStdDev(_preprocessedmat,_vchannelmean,_vchannelstdev);
        _preprocessedmat = (_preprocessedmat - _vchannelmean.at<const double>(0)) / (3.0*_vchannelstdev.at<const double>(0));
        _crops[i] = cvmat2dlibmatrix(_preprocessedmat);
    }
    dlib::matrix<float,0,1> _description = dlib::mean(dlib::mat(net(_crops)));*/

    return dlib::toMat(_description).reshape(1,1).clone();
}

Mat DlibWhalesRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    return getImageDescriptionByLayerName(_img,cv::String(),_error);
}

void DlibWhalesRecognizer::predict(InputArray src, Ptr<PredictCollector> collector, int *_error) const
{
    cv::Mat _description = getImageDescription(src.getMat(),_error);
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
