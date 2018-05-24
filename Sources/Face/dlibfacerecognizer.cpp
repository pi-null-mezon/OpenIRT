#include "dlibfacerecognizer.h"

namespace cv { namespace imgrec {

DlibFaceRecognizer::DlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(0,0),3,true,_disttype,_threshold) // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
{
    try {
        dlibfacedet = dlib::get_frontal_face_detector();
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_faceshapemodelfile.c_str()) >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_facedescriptormodelfile.c_str()) >> net;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
}

Mat DlibFaceRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname) const
{
    cv::String _str = _blobname; // to suppress 'unused variable' compiler warning
    // Prepare image
    dlib::matrix<dlib::rgb_pixel> _facechip = __extractface(preprocessImageForCNN(_img, getInputSize(), getInputChannels(), getCropInput()));

    /*cv::Mat _viewmat(num_rows(_facechip), num_columns(_facechip), CV_8UC3, image_data(_facechip));
    cv::imshow("Input of DLIB",_viewmat);
    cv::waitKey(1);*/

    // Get description
    dlib::matrix<float,0,1> _facedescription = net(_facechip);
    // Perform forward propagation
    return dlib::toMat(_facedescription).reshape(1,1).clone();
}

Mat DlibFaceRecognizer::getImageDescription(const Mat &_img) const
{
    return getImageDescriptionByLayerName(_img,cv::String());
}

void DlibFaceRecognizer::predict(InputArray src, Ptr<PredictCollector> collector) const
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

dlib::matrix<dlib::rgb_pixel> DlibFaceRecognizer::__extractface(const Mat &_inmat) const
{
    cv::Mat _rgbmat = _inmat;
    cv::Mat _graymat;
    cv::cvtColor(_rgbmat, _graymat, CV_BGR2GRAY);

    dlib::cv_image<unsigned char> _graycv_image(_graymat);
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_rgbmat);

    dlib::rectangle _facerect(_inmat.cols,_inmat.rows);
    std::vector<dlib::rectangle> _facerects = dlibfacedet(_graycv_image);
    if(_facerects.size() > 0) {
        _facerect = _facerects[0];
    }
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,150,0.25), _facechip);

    return _facechip;
}

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_faceshapemodelfile, const String &_facedescriptormodelfile, DistanceType _disttype, double _threshold)
{
    return makePtr<DlibFaceRecognizer>(_faceshapemodelfile,_facedescriptormodelfile,_disttype,_threshold);
}

}
}
