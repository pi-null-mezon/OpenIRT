#include "googlenetrecognizer.h"

#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>

#include <fstream>

using namespace std;
using namespace cv;
using namespace imgrec;

#define RES_PATH "C:/Programming/3rdParties/Caffe/models/bvlc_googlenet"

#define PICTURE_FILE "C:/Programming/3rdParties/opencv321/opencv_contrib/modules/dnn/samples/space_shuttle.jpg"
#define LABELINFO_FILE "C:/Programming/3rdParties/opencv321/opencv_contrib/modules/dnn/samples/synset_words.txt"

template <typename T>
std::string real2str(T x, uint precision=2)
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
        i += precision;
    } else {
        i -= 1;
    }
    return std::string(_fullrepres.begin(), _fullrepres.begin() + i);
}

std::vector<String> readClassNames(const char *filename = LABELINFO_FILE)
{
    std::vector<String> classNames;

    std::ifstream fp(filename);
    if (!fp.is_open())
    {
        std::cerr << "File with classes labels not found: " << filename << std::endl;
        exit(-1);
    }

    std::string name;
    while (!fp.eof())
    {
        std::getline(fp, name);
        if (name.length())
            classNames.push_back( name.substr(name.find(' ')+1) );
    }

    fp.close();
    return classNames;
}

int main(int argc, char *argv[])
{

    Ptr<CNNImageRecognizer> _ptr = createGoogleNetRecognizer( String(RES_PATH)+"/bvlc_googlenet.prototxt",
                                                           String(RES_PATH)+"/bvlc_googlenet.caffemodel",
                                                           DistanceType::Euclidean,
                                                           DBL_MAX );
    std::vector<String> _vlabels = readClassNames();

    /*
    Mat _img = imread(PICTURE_FILE, cv::IMREAD_UNCHANGED);
    Mat _dscr = _ptr->getImageDescription(_img);

    float *_data = _dscr.ptr<float>(0);
    int _label = std::max_element(_data, _data+_dscr.total()) - _data;
    std::string _labelinfo = _vlabels[_label];
    std::cout << std::endl << "Predicted label: " << _labelinfo << " (prob: " << _data[_label]*100.0 << " %)"  << std::endl;
    */

    cv::VideoCapture _videocap;

    if(_videocap.open(0)) {

        cv::Mat frame;
        int64 _tickmark1 = cv::getTickCount(), _tickmark2;
        while(_videocap.read(frame)) {

            if(!frame.empty()) {
                Mat _dscr = _ptr->getImageDescriptionByLayerName(frame, "prob");
                float *_data = _dscr.ptr<float>(0);
                int _label = static_cast<int>(std::max_element(_data, _data+_dscr.total()) - _data);
                float _prob = _data[_label]*100.0f;

                String _labelname = _vlabels[_label] + " (" + real2str(_prob).c_str() + " %)";
                cv::putText(frame, _labelname, cv::Point(31,31), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,0,0),1, CV_AA);
                cv::putText(frame, _labelname, cv::Point(30,30), CV_FONT_HERSHEY_SIMPLEX, 0.6, cv::Scalar(0,205 + (_prob - 50),205 - (_prob - 50)),1, CV_AA);


                _tickmark2 = cv::getTickCount();
                double frametime = 1000.0*(_tickmark2 - _tickmark1)/cv::getTickFrequency();
                _tickmark1 = _tickmark2;
                String _frametimestr = real2str(frametime) + " ms";
                cv::putText(frame, _frametimestr, cv::Point(31,frame.rows - 31), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,0,0),1, CV_AA);
                cv::putText(frame, _frametimestr, cv::Point(30,frame.rows - 30), CV_FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255,255,255),1, CV_AA);


                cv::namedWindow("Probe", CV_WINDOW_NORMAL);
                cv::imshow("Probe", frame);
                char option = cv::waitKey(1);
                if(option == 27) {
                    break;
                } else switch(option) {
                    case 'c':
                        _videocap.set(CV_CAP_PROP_SETTINGS,0.0);
                        break;
                }
            }

        }
        _videocap.release();
    }

    return 0;
}
