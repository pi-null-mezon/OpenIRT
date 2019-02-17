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

DlibWhalesRecognizer::DlibWhalesRecognizer(const String &_modelsfiles, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(512,192),CropMethod::NoCrop,ColorOrder::Gray,_disttype,_threshold)
{
    size_t pos1 = _modelsfiles.find(';');
    try {
        std::cout << "  " << _modelsfiles.substr(0,pos1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(0,pos1)) >> rn16_1;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    size_t pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> rn16_2;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> rn16_3;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> rn16_4;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> rn16_5;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> exrn16;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> rn32;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> ex2rn16_1;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    pos1 = pos2;
    pos2 = _modelsfiles.find(';',pos1+1);
    try {
        std::cout << "  " << _modelsfiles.substr(pos1+1,pos2-pos1-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos1+1,pos2-pos1-1)) >> ex2rn16_2;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    /*try {
        std::cout << "  " << _modelsfiles.substr(pos2+1,_modelsfiles.size()-pos2-1) << std::endl;
        dlib::deserialize(_modelsfiles.substr(pos2+1,_modelsfiles.size()-pos2-1)) >> headnet;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }*/
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
    std::vector<cv::Mat> _vdscr;
    dlib::matrix<float,0,1> _dscr1 = rn16_1(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr1));
    dlib::matrix<float,0,1> _dscr2 = rn16_2(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr2));
    dlib::matrix<float,0,1> _dscr3 = rn16_3(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr3));
    dlib::matrix<float,0,1> _dscr4 = rn16_4(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr4));
    dlib::matrix<float,0,1> _dscr5 = rn16_5(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr5));
    dlib::matrix<float,0,1> _dscr6 = exrn16(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr6));
    dlib::matrix<float,0,1> _dscr7 = rn32(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr7));
    dlib::matrix<float,0,1> _dscr8 = ex2rn16_1(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr8));
    dlib::matrix<float,0,1> _dscr9 = ex2rn16_2(cvmat2dlibmatrix(_preprocessedmat));
    _vdscr.push_back(dlib::toMat(_dscr9));

    cv::Mat _dscr;
    cv::merge(_vdscr,_dscr);
    return _dscr.reshape(1,1);

    dlib::matrix<float,0,1> _headdscr = headnet(cvmat2dlibmatrix(_dscr.reshape(1,1)));
    return dlib::toMat(_headdscr).clone().reshape(1,1);

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
    dlib::matrix<float,0,1> _description = dlib::mean(dlib::mat(net_one_1(_crops)));
    return dlib::toMat(_description).clone().reshape(1,1);*/
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

Ptr<CNNImageRecognizer> createDlibWhalesRecognizer(const String &_modelsfiles, DistanceType _disttype, double _threshold)
{
    return makePtr<DlibWhalesRecognizer>(_modelsfiles,_disttype,_threshold);
}

}
}
