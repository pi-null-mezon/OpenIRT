#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include "spoofingattackdetector.h"

using namespace std;

template <typename T>
std::string real2str(T x, uint precision=1);

const cv::String keys =
                        "{help h   |      | print help}"
                        "{url u    |      | use URL as video source}"
                        "{file f   |      | use videofile as video source}"
                        "{dev v    | 0    | use videodevice as video source}"
                        "{cols     | 640  | horizontal resolution for video device}"
                        "{rows     | 480  | vertical resolution for video device}"
                        "{delay    | 1    | delay for cv::imshow in ms}";

int main(int _argc, char **_argv)
{
    cv::CommandLineParser cmdargsparser(_argc, _argv, keys);
    cmdargsparser.about(cv::String(APP_NAME) + " v." + cv::String(APP_VERSION));
    if(cmdargsparser.has("help"))  {
       cmdargsparser.printMessage();
       return 0;
    }

    cv::VideoCapture videocapture;

    if(cmdargsparser.has("url")) {
        cv::String _urlstr = cmdargsparser.get<cv::String>("url");
        if(videocapture.open(_urlstr) == false) {
            cerr << "Could not open url " << _urlstr;
            return -1;
        }
    } else if(cmdargsparser.has("file")) {
        cv::String _filename = cmdargsparser.get<cv::String>("file");
        if(videocapture.open(_filename) == false) {
            cerr << "Could not open videofile " << _filename;
            return -1;
        }
    } else {
        if(videocapture.open(cmdargsparser.get<int>("dev")) == false) {
            cerr << "Could not open videodevice " << cmdargsparser.get<int>("dev");
            return -1;
        } else {
            videocapture.set(cv::CAP_PROP_FRAME_WIDTH, cmdargsparser.get<int>("cols"));
            videocapture.set(cv::CAP_PROP_FRAME_HEIGHT, cmdargsparser.get<int>("rows"));
        }
    }

    cv::Ptr<cv::oirt::CNNImageClassifier> _ptr;

    if(videocapture.isOpened()) {
        cv::Mat _frame;
        int64 _to = cv::getTickCount(), _tn;
        double _fps;
        cv::namedWindow(APP_NAME, cv::WINDOW_NORMAL);
        /*_ptr = cv::oirt::FaceAgeClassifier::createCNNImageClassifier( cv::String("C:/Programming/3rdParties/Caffe/models/FaceAge/deploy_age.prototxt"),
                                                                      cv::String("C:/Programming/3rdParties/Caffe/models/FaceAge/age_net.caffemodel"),
                                                                      cv::String("C:/Programming/3rdParties/DLib/models/shape_predictor_5_face_landmarks.dat"));*/
        _ptr = cv::oirt::SpoofingAttackDetector::createSpoofingAttackDetector("./","shape_predictor_5_face_landmarks.dat");

        int delayms = cmdargsparser.get<int>("delay");
        int error = 0;
        std::vector<float> _vclassprob;
        while(videocapture.read(_frame)) {                
            if(_ptr->getColorOrder() == cv::oirt::RGB)
                _ptr->predict(_frame.clone(),_vclassprob,&error);
            else
                _ptr->predict(_frame,_vclassprob,&error);
            if(error == 0) {
                for(size_t j = 0; j < _vclassprob.size(); ++j) {
                    cv::String _predictionstr = std::string("label '") + _ptr->getLabelInfo(j) + std::string("': ") + real2str(_vclassprob[j],3);
                    cv::putText(_frame, _predictionstr, cv::Point(15,20+20*j), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0),1,cv::LINE_AA);
                    cv::putText(_frame, _predictionstr, cv::Point(14,19+20*j), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,55+200*_vclassprob[j],0),1,cv::LINE_AA);
                }
            } else {
                cv::String _errorstr = _ptr->getErrorInfo(error);
                cv::putText(_frame, _errorstr, cv::Point(15,20), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0),1,cv::LINE_AA);
                cv::putText(_frame, _errorstr, cv::Point(14,19), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,255),1,cv::LINE_AA);
            }

            _tn = cv::getTickCount();
            _fps = cv::getTickFrequency()/(double)(_tn - _to);
            _to = _tn;
            cv::String _frametime_ms = real2str(1000.0/_fps) + " ms (press escape to exit)";
            cv::putText(_frame,_frametime_ms, cv::Point(14,_frame.rows - 19),cv::FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,0,0),1,cv::LINE_AA);
            cv::putText(_frame,_frametime_ms, cv::Point(15,_frame.rows - 20),cv::FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(255,255,255),1,cv::LINE_AA);
            cv::imshow(APP_NAME, _frame);

            char _key= cv::waitKey(delayms);
            if(_key == 27) {// escape pressed
                break;
            } else switch(_key) {

                case 's':
                    videocapture.set(cv::CAP_PROP_SETTINGS,0.0);
                    break;
            }
        }
    }    

    return 0;
}

template <typename T>
std::string real2str(T x, uint precision)
{
    if(x == DBL_MAX)
        return "DBL_MAX";

    std::string _fullrepres = std::to_string(x);
    size_t i;
    for(i = 0; i < _fullrepres.size(); ++i) {
        if(_fullrepres[i] == '.')
            break;
    }
    if(precision > 0) {
        i += precision + 1;
    } else {
        i -= 1;
    }
    return std::string(_fullrepres.begin(), _fullrepres.begin() + i);
}
