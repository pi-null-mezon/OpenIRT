#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include "googlenetrecognizer.h"

using namespace std;
using namespace imgrec;

template <typename T>
std::string real2str(T x, uint precision=1);

const cv::String keys =
                        "{help h   |     | print help}"
                        "{url u    |     | use URL as video source}"
                        "{vfile vf |     | use videofile as video source}"
                        "{vdev vd  |0    | use videodevice as video source}"
                        "{thresh t |2.0  | distance threshold for recognition}"
                        "{rows     |360  | vertical resolution for video device}"
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

    Ptr<CNNImageRecognizer> _ptr;

    if(videocapture.isOpened()) {
        cv::Mat _frame;
        int64 _to = cv::getTickCount(), _tn;
        double _fps;
        cv::namedWindow(APP_NAME, CV_WINDOW_NORMAL);
        int plbl;
        double dist;
        _ptr = createGoogleNetRecognizer( String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet")+"/bvlc_googlenet.prototxt",
                                                                  String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet")+"/bvlc_googlenet.caffemodel",
                                                                  DistanceType::Cosine,
                                                                  DBL_MAX );
        _ptr->ImageRecognizer::load("Memorized_labels_for_recognizer.yml");

        while(videocapture.read(_frame)) {


            char _key= cv::waitKey(1);
            if(_key == 27) {// escape pressed
                break;
            } else switch(_key) {

                case 'c':
                    videocapture.set(CV_CAP_PROP_SETTINGS,0.0);
                    break;

                /*case 's': {
                    cout << endl << "Enter class label: ";
                    int _label;
                    cin >> _label;
                    cout << endl << "And label info: ";
                    string name;
                    cin.ignore(); // removes newline symbol from the input stream, so getline will block untill input will be available
                    getline(cin,name);
                    cv::String _labelInfo(name);
                    cout << "label: [" << _label     << "]" << endl
                         << " info: [" << _labelInfo << "]" << endl;
                    std::vector<Mat> _vimg;
                    _vimg.push_back(_frame);
                    std::vector<int> _vlbl;
                    _vlbl.push_back(_label);
                    _ptr->update(_vimg,_vlbl);
                    _ptr->setLabelInfo(_label,_labelInfo);
                    } break;*/

                case 's': {
                    cout << endl << "Provide label info: ";
                    string name;
                    getline(cin,name);
                    cv::String _labelInfo(name);
                    int _label = _ptr->nextfreeLabel();
                    cout << "label: [" << _label     << "]" << endl
                         << " info: [" << _labelInfo << "]" << endl;
                    std::vector<Mat> _vimg;
                    _vimg.push_back(_frame);
                    std::vector<int> _vlbl;
                    _vlbl.push_back(_label);
                    _ptr->update(_vimg,_vlbl);
                    _ptr->setLabelInfo(_label,_labelInfo);
                    } break;

                /*case 'r': {
                    if(_ptr->empty()) {
                        cout << "Recognizer is not trained yet!" << endl ;
                    } else {
                        _ptr->predict(_frame,plbl,dist);
                        cout << "Recognized as: " << _ptr->getLabelInfo(plbl) << "; distidence: " << dist << endl;
                    }
                } break;*/

            }

            if(_ptr->empty()) {
                cv::String _labelInfo = "Recognizer is not trained yet!";
                cv::putText(_frame,_labelInfo, cv::Point(14,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.75,cv::Scalar(0,0,0),1,CV_AA);
                cv::putText(_frame,_labelInfo, cv::Point(15,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.75,cv::Scalar(0,0,255),1,CV_AA);
            } else {
                _ptr->predict(_frame,plbl,dist);
                if(dist < cmdargsparser.get<double>("thresh")) {
                    cv::String _labelInfo = _ptr->getLabelInfo(plbl) + ", dist: " + real2str(dist,2);
                    cv::putText(_frame,_labelInfo, cv::Point(14,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.75,cv::Scalar(0,0,0),1,CV_AA);
                    cv::putText(_frame,_labelInfo, cv::Point(15,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.75,cv::Scalar(0,255,0),1,CV_AA);
                }
            }

            _tn = cv::getTickCount();
            _fps = cv::getTickFrequency()/(double)(_tn - _to);
            _to = _tn;
            cv::String _frametime_ms = real2str(1000.0/_fps) + " ms";
            cv::putText(_frame,_frametime_ms, cv::Point(14,19),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,0,0),1,CV_AA);
            cv::putText(_frame,_frametime_ms, cv::Point(15,20),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(255,255,255),1,CV_AA);
            cv::imshow(APP_NAME, _frame);
        }
    }
    _ptr->ImageRecognizer::save("Memorized_labels_for_recognizer.yml");

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
