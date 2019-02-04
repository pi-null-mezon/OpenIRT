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

const cv::String options = "{help h       |       | show app's help}"
                           "{inputdir i   |       | set input directory in which subdirs will be searched}"
                           "{outputfile o |       | set output filename}"
                           "{model m      |       | set model filename}"
                           "{aggavg       | false | aggregate descriptions for label by averaging}"
                           "{aggmed       | false | aggregate descriptions for label by median}"
                           "{hflip        | false | add description of a horizontal flipped copy of image}"
                           "{vflip        | false | add description of a vertical flipped copy of image}";

cv::Mat getMedianDscr(const std::vector<cv::Mat> &_vlbldscr);

cv::Mat getAvgDscr(const std::vector<cv::Mat> &_vlbldscr);

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    cv::CommandLineParser _cmdparser(argc,argv,options);
    _cmdparser.about( QString("%1 v.%2").arg(APP_NAME,APP_VERSION).toUtf8().constData() );
    if(_cmdparser.has("help") || (argc == 1)) {
        _cmdparser.printMessage();
        return 0;
    }
    if(_cmdparser.has("inputdir") == false) {
        qInfo("You have not provide inputdir name! Abort...");
        return 1;
    }
    if(_cmdparser.has("outputfile") == false) {
        qInfo("You have not provide outputfile name! Abort...");
        return 2;
    }
    if(_cmdparser.has("model") == false) {
        qInfo("You have not provide model file name! Abort...");
        return 3;
    } else {
        QFileInfo _fi(_cmdparser.get<cv::String>("model").c_str());
        if(!_fi.exists()) {
            qInfo("Model file you have provided does not exists! Abort...");
            return 4;
        }
    }

    QDir _dir(_cmdparser.get<cv::String>("inputdir").c_str());
    if(_dir.exists() == false) {
        qInfo("Input directory does not exist! Abort...");
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
    const bool _aggavg = _cmdparser.get<bool>("aggavg");
    const bool _aggmed = _cmdparser.get<bool>("aggmed");
    const bool _hflip  = _cmdparser.get<bool>("hflip");
    const bool _vflip  = _cmdparser.get<bool>("vflip");
    QStringList _extensions;
    _extensions << "*.png" << "*.jpeg" << "*.bmp" << "*.jpg";
    for(int i = 0; i < _lsubdirname.size(); ++i) {
        QString _labelstr = _lsubdirname.at(i);
        qInfo("  Label number %d - name %s", i, _labelstr.toUtf8().constData());
        QDir _subdir(_dir.absolutePath().append("/%1").arg(_lsubdirname.at(i)));        
        QStringList _lfilename = _subdir.entryList(_extensions, QDir::Files | QDir::NoDotAndDotDot);

        if(_lfilename.size() > 0) {
            std::vector<cv::Mat> _vlbldscr;
            _vlbldscr.reserve( _lfilename.size());
            for(int j = 0; j < _lfilename.size(); ++j) {
                QString _filename = _subdir.absoluteFilePath(_lfilename.at(j));
                cv::Mat _imgmat = cv::imread(_filename.toLocal8Bit().constData(), cv::IMREAD_UNCHANGED);
                _vlbldscr.push_back(_ptr->getImageDescription(_imgmat));
                if(_hflip) {
                    cv::flip(_imgmat,_imgmat,1);
                    _vlbldscr.push_back(_ptr->getImageDescription(_imgmat));
                }
                if(_vflip) {
                    cv::flip(_imgmat,_imgmat,0);
                    _vlbldscr.push_back(_ptr->getImageDescription(_imgmat));
                }
            }
            if(_aggavg && (_lfilename.size() < 1000)) {
                _ptr->addKnownDescription(getAvgDscr(_vlbldscr),i);
            } else if(_aggmed && (_lfilename.size() < 1000)){
                _ptr->addKnownDescription(getMedianDscr(_vlbldscr),i);
            } else {
                for(size_t k = 0; k < _vlbldscr.size(); ++k) {
                    _ptr->addKnownDescription(_vlbldscr[k],i);
                }
            }
            _ptr->setLabelInfo(i,_labelstr.toUtf8().constData());
        }
    }

    cv::String _outputfilename = _cmdparser.get<cv::String>("outputfile");
    qInfo("Please wait untill labels will be saved in %s", _outputfilename.c_str());
    _ptr->ImageRecognizer::save(_outputfilename);
    qInfo("  Done");
    return 0;
}

cv::Mat getMedianDscr(const std::vector<cv::Mat> &_vlbldscr)
{
    if(_vlbldscr.size() == 1 || _vlbldscr.size() == 2)
        return _vlbldscr[0];

    cv::Mat _tmpdscrmat = cv::Mat::zeros(_vlbldscr[0].rows,_vlbldscr[0].cols,_vlbldscr[0].type());
    for(size_t k = 0; k < _tmpdscrmat.total(); ++k) {
        std::vector<float> _vtmp(_vlbldscr.size(),0.0f);
        for(size_t n = 0; n < _vtmp.size(); ++n) {
            _vtmp[n] = _vlbldscr[n].at<const float>(k);
        }
        std::nth_element(_vtmp.begin(),_vtmp.begin()+_vtmp.size()/2,_vtmp.end());
        _tmpdscrmat.at<float>(k) = _vtmp[_vtmp.size()/2];
    }
    return _tmpdscrmat;
}

cv::Mat getAvgDscr(const std::vector<cv::Mat> &_vlbldscr)
{
    if(_vlbldscr.size() == 1)
        return _vlbldscr[0];

    cv::Mat _tmpdscrmat = cv::Mat::zeros(_vlbldscr[0].rows,_vlbldscr[0].cols,_vlbldscr[0].type());
    for(size_t k = 0; k < _vlbldscr.size(); ++k) {
        _tmpdscrmat += _vlbldscr[k];
    }
    return _tmpdscrmat / _vlbldscr.size();
}
