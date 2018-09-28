#include "dialyzerrecognizer.h"

#include <opencv2/highgui.hpp>

namespace cv { namespace oirt {

DialyzerRecognizer::DialyzerRecognizer(const String &_modelfile, DistanceType _disttype, double _threshold) :
    CNNImageRecognizer(cv::Size(360,360),Inside,ColorOrder::RGB,_disttype,_threshold) // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
{
    try {
        dlib::deserialize(_modelfile.c_str()) >> net;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }

    // Declare errors
    // errorsInfo[1] = "Can not find dialyzer!"; - example of possible error
}

Mat DialyzerRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    cv::String _str = _blobname; // to suppress 'unused variable' compiler warning
    cv::Mat _rgbmat = preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput());

    // Let's mask periferials
    int _thresh = _rgbmat.rows/7;
    for(int y = 0; y < _rgbmat.rows; ++y) {
        unsigned char *_p = _rgbmat.ptr<unsigned char>(y);
        for(int x = 0; x < _rgbmat.cols; ++x) {
            float _multiplier = (1.0f - std::abs(y - _rgbmat.rows/2.0f)/(_rgbmat.rows/2.0f));
            _multiplier = _multiplier*_multiplier;
            //if(std::abs(y - _rgbmat.rows/2) > _thresh) {
                _p[3*x]   *= _multiplier;
                _p[3*x+1] *= _multiplier;
                _p[3*x+2] *= _multiplier;
            //}
        }
    }

	dlib::cv_image<dlib::rgb_pixel> _iimg(_rgbmat);
	dlib::matrix<dlib::rgb_pixel> _preprocessed;
	dlib::assign_image(_preprocessed,_iimg);

    cv::Mat _viewmat(num_rows(_preprocessed), num_columns(_preprocessed), CV_8UC3, image_data(_preprocessed));
    cv::imshow("Input of DLIB",_viewmat);
    cv::waitKey(1);

	dlib::matrix<float,0,1> _facedescription = net(_preprocessed);
	return dlib::toMat(_facedescription).reshape(1,1).clone();
}

Mat DialyzerRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    return getImageDescriptionByLayerName(_img,cv::String(),_error);
}

void DialyzerRecognizer::predict(InputArray src, Ptr<PredictCollector> collector, int *_error) const
{
    cv::Mat _description = getImageDescription(src.getMat(),_error);
    collector->init(v_labels.size());
    for (size_t sampleIdx = 0; sampleIdx < v_labels.size(); sampleIdx++) {
        if(v_whitelist[sampleIdx] != 0x00) { // only whitelisted values
            double distance = DBL_MAX;
            switch(getDistanceType()) {
            case DistanceType::Euclidean:
                distance = euclideanDistance(v_descriptions[sampleIdx], _description);
                break;
            case DistanceType::Cosine:
                distance =  cosineDistance(v_descriptions[sampleIdx], _description);
                break;
            }
            if( !collector->collect(v_labels[sampleIdx], distance) ) {
                return;
            }
        }
    }
}

Ptr<CNNImageRecognizer> createDialyzerRecognizer(const String &_modelfile, DistanceType _disttype, double _threshold)
{
    return makePtr<DialyzerRecognizer>(_modelfile,_disttype,_threshold);
}

}
}
