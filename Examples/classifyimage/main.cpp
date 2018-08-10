#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include "faceageclassifier.h"

using namespace std;

template <typename T>
std::string real2str(T x, uint precision=1);

const cv::String keys =
                        "{help h   |     | print help}"
                        "{url u    |     | use URL as video source}"
                        "{vfile vf |     | use videofile as video source}"
                        "{vdev vd  |0    | use videodevice as video source}"
                        "{rows     |480  | vertical resolution for video device}"
                        "{cols     |640  | horizontal resolution for video device}";

int main(int _argc, char **_argv)
{
    cv::CommandLineParser cmdargsparser(_argc, _argv, keys);
    cmdargsparser.about(cv::String(APP_NAME) + " v." + cv::String(APP_VERSION));
    if(cmdargsparser.has("help"))   {
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
    } else if(cmdargsparser.has("vfile")) {
        cv::String _filename = cmdargsparser.get<cv::String>("vfile");
        if(videocapture.open(_filename) == false) {
            cerr << "Could not open videofile " << _filename;
            return -1;
        }
    } else {
        if(videocapture.open(cmdargsparser.get<int>("vdev")) == false) {
            cerr << "Could not open videodevice " << cmdargsparser.get<int>("vdev");
            return -1;
        } else {
            videocapture.set(CV_CAP_PROP_FRAME_WIDTH, cmdargsparser.get<int>("cols"));
            videocapture.set(CV_CAP_PROP_FRAME_HEIGHT, cmdargsparser.get<int>("rows"));
        }
    }

    cv::Ptr<cv::oirt::CNNImageClassifier> _ptr;

    if(videocapture.isOpened()) {
        cv::Mat _frame;
        int64 _to = cv::getTickCount(), _tn;
        double _fps;
        cv::namedWindow(APP_NAME, CV_WINDOW_NORMAL);
        _ptr = cv::oirt::FaceAgeClassifier::createCNNImageClassifier( cv::String("C:/Programming/3rdParties/Caffe/models/FaceAge/deploy_age.prototxt"),
                                                                      cv::String("C:/Programming/3rdParties/Caffe/models/FaceAge/age_net.caffemodel"),
                                                                      cv::String("C:/Programming/3rdParties/DLib/models/shape_predictor_5_face_landmarks.dat"));


        int label = -1;
        double conf = 0;
        while(videocapture.read(_frame)) {

            _ptr->predict(_frame,label,conf);
            cv::String _predictionstr = std::string("label: ") + std::to_string(label) + std::string("; conf.: ") + real2str(conf,3) + std::string(" >> ") + _ptr->getLabelInfo(label);
            std::cout << _predictionstr.c_str() << std::endl;
            cv::putText(_frame, _predictionstr, cv::Point(15,20), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0),1,CV_AA);
            cv::putText(_frame, _predictionstr, cv::Point(14,19), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0),1,CV_AA);


            _tn = cv::getTickCount();
            _fps = cv::getTickFrequency()/(double)(_tn - _to);
            _to = _tn;
            cv::String _frametime_ms = real2str(1000.0/_fps) + " ms (press escape to exit)";
            cv::putText(_frame,_frametime_ms, cv::Point(14,_frame.rows - 19),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,0,0),1,CV_AA);
            cv::putText(_frame,_frametime_ms, cv::Point(15,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(255,255,255),1,CV_AA);
            cv::imshow(APP_NAME, _frame);

            char _key= cv::waitKey(1);
            if(_key == 27) {// escape pressed
                break;
            } else switch(_key) {

                case 's':
                    videocapture.set(CV_CAP_PROP_SETTINGS,0.0);
                    break;
            }
        }
    }    

    return 0;
}

template <typename T>
std::string real2str(T x, uint precision)
{
    if(x == DBL_MAX) {
        return "DBL_MAX";
    }
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
