#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include <QDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

//#include "squeezenetimagenetrecognizer.h"
//#include "googlenetrecognizer.h"
#include "dlibwhalesrecognizer.h"
//#include "furniturerecognizer.h"

const cv::String options = "{help h       |        | show app's help}"
                           "{inputdir i   |        | set input directory in which subdirs will be searched}"
                           "{outputfile o |        | set output filename}"
                           "{model m      |        | set model filename}"
                           "{visualize v  |  true  | set visualization option}"
                           "{aggavg       |  false | aggregate descriptions for label by averaging}"
                           "{aggmed       |  false | aggregate descriptions for label by median}";

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    cv::CommandLineParser _cmdparser(argc,argv,options);
    _cmdparser.about( QString("%1 v.%2").arg(APP_NAME,APP_VERSION).toUtf8().constData() );
    if(_cmdparser.has("help")) {
        _cmdparser.printMessage();
        return 0;
    }
    if(_cmdparser.has("inputdir") == false) {
        qInfo("You have not provide inputdir name. Abort...");
        return 1;
    }
    if(_cmdparser.has("outputfile") == false) {
        qInfo("You have not provide outputfile name. Abort...");
        return 2;
    }
    if(_cmdparser.has("model") == false) {
        qInfo("You have not provide model file name. Abort...");
        return 3;
    } else {
        QFileInfo _fi(_cmdparser.get<cv::String>("model").c_str());
        if(!_fi.exists()) {
            qInfo("Model file name you have provided does not exists. Abort...");
            return 4;
        }
    }

    QDir _dir(_cmdparser.get<cv::String>("inputdir").c_str());
    if(_dir.exists() == false) {
        qInfo("Input directory is not existed. Abort...");
        return 5;
    }

    qInfo("Step_1 - Uploading recognizer resources from HDD...");
    /*cv::Ptr<oirt::CNNImageRecognizer> _ptr = cv::oirt::createSqueezeNetImageNetRecognizer(cv::String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.prototxt"),
                                                                                          cv::String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.caffemodel"));*/

    /*cv::Ptr<cv::oirt::CNNImageRecognizer> _ptr = cv::oirt::createGoogleNetRecognizer( cv::String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.prototxt"),
                                                                                        cv::String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.caffemodel") );*/

    cv::Ptr<cv::oirt::CNNImageRecognizer> _ptr = cv::oirt::createDlibWhalesRecognizer(_cmdparser.get<cv::String>("model"));

    /*cv::Ptr<cv::oirt::CNNImageRecognizer> _ptr = cv::oirt::createFurnitureRecognizer(cv::String("/home/alex/Fastdata/Kaggle/Furniture/Metricbench/net.dat"));*/

    qInfo("Step_2 - Generating descriptions for labels");
    QStringList _lsubdirname = _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    const bool _visualize = _cmdparser.get<bool>("visualize");
    bool _aggavg = _cmdparser.get<bool>("aggavg"), _aggmed = _cmdparser.get<bool>("aggmed");
    for(int i = 0; i < _lsubdirname.size(); ++i) {
        QString _labelstr = _lsubdirname.at(i);
        qInfo("   Label number %d - name %s", i, _labelstr.toUtf8().constData());
        QDir _subdir(_dir.absolutePath().append("/%1").arg(_lsubdirname.at(i)));
        QStringList _extensions;
        _extensions << "*.png" << "*.jpeg" << "*.bmp" << "*.jpg";
        QStringList _lfilename = _subdir.entryList(_extensions, QDir::Files | QDir::NoDotAndDotDot);

        if((_aggavg || _aggmed) && (_lfilename.size() < 1000)) {
            std::vector<cv::Mat> _vlbldscr;
            _vlbldscr.reserve( _lfilename.size());
            for(int j = 0; j < _lfilename.size(); ++j) {
                QString _filename = _subdir.absoluteFilePath(_lfilename.at(j));
                _vlbldscr.push_back(_ptr->getImageDescription(cv::imread(_filename.toLocal8Bit().constData(), cv::IMREAD_UNCHANGED)));
            }

            if(_vlbldscr.size() > 0) {
                cv::Mat _tmpdscrmat = cv::Mat::zeros(_vlbldscr[0].rows,_vlbldscr[0].cols,_vlbldscr[0].type());
                if(_aggavg) {
                    for(size_t k = 0; k < _vlbldscr.size(); ++k) {
                        _tmpdscrmat += _vlbldscr[k];
                    }
                    _tmpdscrmat /= _vlbldscr.size();
                } else {
                    for(size_t k = 0; k < _tmpdscrmat.total(); ++k) {
                        std::vector<float> _vtmp(_vlbldscr.size(),0.0f);
                        for(size_t n = 0; n < _vtmp.size(); ++n) {
                            _vtmp[n] = _vlbldscr[n].at<const float>(k);
                        }
                        std::nth_element(_vtmp.begin(),_vtmp.begin()+_vtmp.size()/2,_vtmp.end());
                        _tmpdscrmat.at<float>(k) = _vtmp[_vtmp.size()/2];
                    }
                }
                _ptr->addKnownDescription(_tmpdscrmat,i);
            }
        } else {
            for(int j = 0; j < _lfilename.size(); ++j) {
                QString _filename = _subdir.absoluteFilePath(_lfilename.at(j));
                _ptr->addKnownDescription(_ptr->getImageDescription(cv::imread(_filename.toLocal8Bit().constData(), cv::IMREAD_UNCHANGED)),i);
                /*std::vector<cv::Mat> _vmat;
                _vmat.push_back(cv::imread(_filename.toLocal8Bit().constData(), cv::IMREAD_UNCHANGED));
                std::vector<int> _vlbl;
                _vlbl.push_back(i);
                _ptr->update(_vmat,_vlbl,_visualize); // maybe it is not fastest way, but it is pretty interactive instead*/
            }
        }
        if(_lfilename.size() > 0) {
            _ptr->setLabelInfo(i,_labelstr.toUtf8().constData());
        }
    }

    cv::String _outputfilename = _cmdparser.get<cv::String>("outputfile");
    qInfo("Saving descriptions in %s", _outputfilename.c_str());
    _ptr->ImageRecognizer::save(_outputfilename);
    return 0;
}
