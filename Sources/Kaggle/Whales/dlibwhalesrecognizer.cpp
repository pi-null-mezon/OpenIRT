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
    std::string _filename;

    size_t pos1 = _modelsfiles.find(';');
    try {
        _filename = _modelsfiles.substr(0,pos1);
        std::cout << "  " << _filename << std::endl;
        dlib::deserialize(_filename) >> rn32;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    size_t pos2 = _modelsfiles.find(';',pos1+1);
    try {
        _filename = _modelsfiles.substr(pos1+1,pos2-pos1-1);
        std::cout << "  " << _filename << std::endl;
        dlib::deserialize(_filename) >> exrn16;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    ex2rn16.resize(2);
    for(size_t i = 0; i < ex2rn16.size(); ++i) {
        pos1 = pos2;
        pos2 = _modelsfiles.find(';',pos1+1);
        try {
            _filename = _modelsfiles.substr(pos1+1,pos2-pos1-1);
            std::cout << "  " << _filename << std::endl;
            dlib::deserialize(_filename) >> ex2rn16[i];
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    /*drn16.resize(2);
    for(size_t i = 0; i < drn16.size(); ++i) {
        pos1 = pos2;
        pos2 = _modelsfiles.find(';',pos1+1);
        try {
            _filename = _modelsfiles.substr(pos1+1,pos2-pos1-1);
            std::cout << "  " << _filename << std::endl;
            dlib::deserialize(_filename) >> drn16[i];
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }

    vrn16.resize(2);
    for(size_t i = 0; i < vrn16.size(); ++i) {
        pos1 = pos2;
        pos2 = _modelsfiles.find(';',pos1+1);
        try {
            _filename = _modelsfiles.substr(pos1+1,pos2-pos1-1);
            std::cout << "  " << _filename << std::endl;
            dlib::deserialize(_filename) >> vrn16[i];
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }*/

    rn16.resize(5);
    for(size_t i = 0; i < rn16.size(); ++i) {
        pos1 = pos2;
        pos2 = _modelsfiles.find(';',pos1+1);
        try {
            _filename = _modelsfiles.substr(pos1+1,pos2-pos1-1);
            std::cout << "  " << _filename << std::endl;
            dlib::deserialize(_filename) >> rn16[i];
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
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
    const dlib::matrix<float> _dlibimgrepresentation = cvmat2dlibmatrix(_preprocessedmat);

    // Get description
    std::vector<cv::Mat> _vdscr;
    for(size_t i = 0; i < rn16.size(); ++i) {
        dlib::matrix<float,0,1> _dscr = rn16[i](_dlibimgrepresentation);
        _vdscr.push_back(dlib::toMat(_dscr).clone());
    }
    for(size_t i = 0; i < ex2rn16.size(); ++i) {
        dlib::matrix<float,0,1> _dscr = ex2rn16[i](_dlibimgrepresentation);
        _vdscr.push_back(dlib::toMat(_dscr).clone());
    }
    /*for(size_t i = 0; i < drn16.size(); ++i) {
        dlib::matrix<float,0,1> _dscr = drn16[i](_dlibimgrepresentation);
        _vdscr.push_back(dlib::toMat(_dscr).clone());
    }
    for(size_t i = 0; i < vrn16.size(); ++i) {
        dlib::matrix<float,0,1> _dscr = vrn16[i](_dlibimgrepresentation);
        _vdscr.push_back(dlib::toMat(_dscr).clone());
    }*/
    dlib::matrix<float,0,1> _dscr1 = exrn16(_dlibimgrepresentation);
    _vdscr.push_back(dlib::toMat(_dscr1));
    dlib::matrix<float,0,1> _dscr2 = rn32(_dlibimgrepresentation);
    _vdscr.push_back(dlib::toMat(_dscr2));

    cv::Mat _tdscr;
    cv::merge(_vdscr,_tdscr);
    return _tdscr.reshape(1,1);

    dlib::matrix<float,0,1> _headdscr = headnet(cvmat2dlibmatrix(_tdscr.reshape(1,1)));
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
