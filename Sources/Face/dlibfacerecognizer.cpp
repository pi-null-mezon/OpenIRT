#include "dlibfacerecognizer.h"

namespace cv { namespace oirt {

DlibFaceRecognizer::DlibFaceRecognizer(const String &_resourcesdirectory, DistanceType _disttype, double _threshold, double _livenessthresh, int _samples) :
    CNNImageRecognizer(cv::Size(0,0),NoCrop,ColorOrder::BGR,_disttype,_threshold), // zeros in Size means that input image will not be changed in size on preprocessing step, it is necessary for the internal face detector
    livenessthresh(_livenessthresh),
    samples(_samples)
{
    ofrtfacedetPtr = cv::ofrt::CNNFaceDetector::createDetector(_resourcesdirectory + "/deploy_lowres.prototxt",
                                                               _resourcesdirectory + "/res10_300x300_ssd_iter_140000_fp16.caffemodel",
                                                               0.5);
    ofrtfacedetPtr->setPortions(1.2f,1.2f);

    try {
        dlib::deserialize(_resourcesdirectory + "/shape_predictor_5_face_landmarks.dat") >> dlibshapepredictor;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    try {
        dlib::deserialize(_resourcesdirectory + "/dlib_face_recognition_resnet_model_v1.dat") >> inet;
    } catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    anets = std::vector<dlib::softmax<dlib::attackmodel::subnet_type>>(5);
    for(size_t i = 0; i < anets.size(); ++i) {
        try {
            dlib::attackmodel _tmpnet;
            dlib::deserialize(_resourcesdirectory + "/net_0_split_" + std::to_string(i) + ".dat") >> _tmpnet;
            anets[i].subnet() = _tmpnet.subnet();
        } catch(const std::exception& e) {
            std::cout << e.what() << std::endl;
        }
    }
    // Define errors
    errorsInfo[2] = "No faces found";
    errorsInfo[3] = "Many faces found";
    errorsInfo[4] = "Spoofing attack";
}

Mat DlibFaceRecognizer::getImageDescriptionByLayerName(const Mat &_img, const String &_blobname, int *_error) const
{
    return getImageDescription(_img,_error);
}

Mat DlibFaceRecognizer::getImageDescription(const Mat &_img, int *_error) const
{
    cv::Mat _dscrmat = cv::Mat::zeros(1,128,CV_32FC1);

    cv::Mat _preprocessedmat = preprocessImageForCNN(_img, getInputSize(), getColorOrder(), getCropInput());
    auto _facesrects = __detectfaces(_preprocessedmat);
    cv::cvtColor(_preprocessedmat,_preprocessedmat,cv::COLOR_BGR2RGB);

    if(_facesrects.size() == 0) {
        if(_error)
            *_error = 2; // no faces
    } else if(_facesrects.size() > 1) {
        if(_error)
            *_error = 3; // many faces
    } else if(_facesrects[0].area() > 0) {
        if(_error)
            *_error = 0;

        const dlib::matrix<dlib::rgb_pixel> _facechip = __extractface(_preprocessedmat,_facesrects[0],150,0.25);

        auto computer = [] (const dlib::matrix<dlib::rgb_pixel> &_facechip,
                            dlib::faceidentitymodel &net,
                            time_t _seed,
                            uint _samples,
                            cv::Mat &_dscr) {
            dlib::rand rnd(_seed);
            std::vector<dlib::matrix<dlib::rgb_pixel>> variants(_samples);
            if(_samples == 1)
                variants[0] = _facechip;
            else
                for(size_t i = 0; i < variants.size(); ++i)
                    variants[i] = dlib::jitter_image(_facechip,rnd);
            dlib::matrix<float,0,1> _facedescription = dlib::mean(dlib::mat(net(variants)));
            float *ptr = _dscr.ptr<float>(0);
            std::memcpy(ptr,&_facedescription(0),128*sizeof(float));
        };

        std::thread *_thread = new std::thread(computer,std::cref(_facechip),std::ref(inet),1,samples,std::ref(_dscrmat));

        // Spoofing control
        if((livenessthresh < 0.9999) && spoofingcontrolenabled) {
            float live = 0;
            for(size_t i = 0; i < anets.size(); ++i) {
                dlib::matrix<float,1,4> p = dlib::mat(anets[i](_facechip));
                live += p(0);
            }
            live /= anets.size();
            //std::cout << "Confidence of liveness: " << live << std::endl;

            if(live < livenessthresh)
                if(_error)
                    *_error = 4; // spoofing attack
        }

        _thread->join();
        delete _thread;

    }
    return _dscrmat;
}

dlib::matrix<dlib::rgb_pixel> DlibFaceRecognizer::__extractface(const Mat &_inmat, const dlib::rectangle &_facerect,  unsigned long _targetsize, double _padding) const
{
    dlib::cv_image<dlib::rgb_pixel> _rgbcv_image(_inmat);
    auto _shape = dlibshapepredictor(_rgbcv_image, _facerect);
    dlib::matrix<dlib::rgb_pixel> _facechip;
    dlib::extract_image_chip(_rgbcv_image, dlib::get_face_chip_details(_shape,_targetsize,_padding), _facechip);
    return _facechip;
}

std::vector<dlib::rectangle> DlibFaceRecognizer::__detectfaces(const Mat &_inmat) const
{
    /*cv::Mat _graymat;
    cv::cvtColor(_inmat, _graymat, cv::COLOR_RGB2GRAY);
    std::vector<dlib::rectangle> _dlibrects = dlibfacedet(dlib::cv_image<unsigned char>(_graymat));*/

    std::vector<cv::Rect> _cvrects = ofrtfacedetPtr->detectFaces(_inmat);
    std::vector<dlib::rectangle> _dlibrects;
    _dlibrects.reserve(_cvrects.size());
    const cv::Rect imgrect(0,0,_inmat.cols,_inmat.rows);
    for(size_t i = 0; i < _cvrects.size(); ++i) {
        const cv::Rect squarerect = makeSquareRect(_cvrects[i]) & imgrect;
        _dlibrects.push_back(dlib::rectangle(squarerect.tl().x,
                                             squarerect.tl().y,
                                             squarerect.br().x,
                                             squarerect.br().y));
    }
    if(_dlibrects.size() > 1) {
        std::sort(_dlibrects.begin(),_dlibrects.end(),[](const dlib::rectangle &lhs, const dlib::rectangle &rhs) {
            return lhs.area() > rhs.area();
        });
    }
    return _dlibrects;
}

Ptr<CNNImageRecognizer> createDlibFaceRecognizer(const String &_resourcesdirectory, DistanceType _disttype, double _threshold, double _livenessthresh, int _samples)
{
    return makePtr<DlibFaceRecognizer>(_resourcesdirectory,_disttype,_threshold,_livenessthresh,_samples);
}

}
}
