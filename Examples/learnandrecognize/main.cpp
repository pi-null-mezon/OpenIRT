#include <iostream>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgproc.hpp>

#include "googlenetrecognizer.h"
#include "resnet50imagenetrecognizer.h"
#include "squeezenetimagenetrecognizer.h"

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

    Ptr<CNNImageRecognizer> _ptr;

    if(videocapture.isOpened()) {
        cv::Mat _frame;
        int64 _to = cv::getTickCount(), _tn;
        double _fps;
        cv::namedWindow(APP_NAME, CV_WINDOW_NORMAL);
        /*_ptr = createGoogleNetRecognizer( String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.prototxt"),
                                          String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.caffemodel") );
        */

        /*_ptr = createResNet50ImageNetRecognizer( String("C:/Programming/3rdParties/Caffe/models/ImageNet-ResNet50/ResNet-50-deploy.prototxt"),
                                                 String("C:/Programming/3rdParties/Caffe/models/ImageNet-ResNet50/ResNet-50-model.caffemodel") );
        */

        _ptr = createSqueezeNetImageNetRecognizer(String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.prototxt"),
                                                  String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.caffemodel") );

        _ptr->ImageRecognizer::load("Memorized_labels_for_recognizer.yml");

        while(videocapture.read(_frame)) {

            char _key= cv::waitKey(1);
            if(_key == 27) {// escape pressed
                break;
            } else switch(_key) {

                case 'c':
                    videocapture.set(CV_CAP_PROP_SETTINGS,0.0);
                    break;

                case 's': {
                    cout << endl << "Provide label info: ";
                    string name;
                    getline(cin,name);
                    cv::String _labelInfo(name);
                    int _label;
                    // Let's check if this labelinfo already in use
                    auto _vlabels = _ptr->getLabelsByString(_labelInfo);
                    if(_vlabels.size() > 0) {
                        cout << "This label info already in use, so this example will be added to existed label" << std::endl;
                        _label =_vlabels[0];
                    } else {
                        _label = _ptr->nextfreeLabel();
                    }
                    cout << "label: [" << _label << "] <->" << " info: [" << _labelInfo << "]" << endl;
                    std::vector<Mat> _vimg;
                    _vimg.push_back(_frame);
                    std::vector<int> _vlbl;
                    _vlbl.push_back(_label);
                    _ptr->update(_vimg,_vlbl);
                    _ptr->setLabelInfo(_label,_labelInfo);

                    _ptr->ImageRecognizer::save("Memorized_labels_for_recognizer.yml");
                    } break;

            }

            if(_ptr->empty()) {
                cv::String _labelInfo = "No descriptions available yet!";
                cv::putText(_frame,_labelInfo, cv::Point(14,30),CV_FONT_HERSHEY_SIMPLEX,0.65,cv::Scalar(0,0,0),1,CV_AA);
                cv::putText(_frame,_labelInfo, cv::Point(15,30),CV_FONT_HERSHEY_SIMPLEX,0.65,cv::Scalar(0,0,255),1,CV_AA);
            } else {               
                std::vector<std::pair<int,double>> _vpredictions = _ptr->recognize(_frame);
                for(size_t _r = 0; _r < _vpredictions.size(); ++_r) {
                    if(_vpredictions[_r].second < cmdargsparser.get<double>("thresh")) {
                        cv::String _labelInfo = std::to_string(_r) + ") " + _ptr->getLabelInfo(_vpredictions[_r].first) + ", dist: " + real2str(_vpredictions[_r].second,2);
                        cv::putText(_frame,_labelInfo, cv::Point(14,20 + (int)_r*20),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,0,0),1,CV_AA);
                        cv::putText(_frame,_labelInfo, cv::Point(15,20 + (int)_r*20),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,255 - (int)_r*25,0),1,CV_AA);
                    }
                }
            }

            _tn = cv::getTickCount();
            _fps = cv::getTickFrequency()/(double)(_tn - _to);
            _to = _tn;
            cv::String _frametime_ms = real2str(1000.0/_fps) + " ms (press escape to exit)";
            cv::putText(_frame,_frametime_ms, cv::Point(14,_frame.rows - 19),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(0,0,0),1,CV_AA);
            cv::putText(_frame,_frametime_ms, cv::Point(15,_frame.rows - 20),CV_FONT_HERSHEY_SIMPLEX,0.5,cv::Scalar(255,255,255),1,CV_AA);
            cv::imshow(APP_NAME, _frame);
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
