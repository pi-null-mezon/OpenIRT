#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include <QDir>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "squeezenetimagenetrecognizer.h"
#include "googlenetrecognizer.h"
#include "dlibwhalesrecognizer.h"

const cv::String options = "{help h       |        | show app's help}"
                           "{inputdir i   |        | set input directory in which subdirs will be searched}"
                           "{outputfile o |        | set output filename}"
                           "{visualize v  |  true  | set visualization option}";

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

    QDir _dir(_cmdparser.get<cv::String>("inputdir").c_str());
    if(_dir.exists() == false) {
        qInfo("Input directory is not existed. Abort...");
        return 3;
    }

    qInfo("Step_1 - Uploading recognizer resources from HDD...");
    /*cv::Ptr<imgrec::CNNImageRecognizer> _ptr = cv::imgrec::createSqueezeNetImageNetRecognizer(cv::String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.prototxt"),
                                                                                          cv::String("C:/Programming/3rdParties/Caffe/models/ImageNet-SqueezeNet/squeezenet_v1.1.caffemodel"));*/

    /*cv::Ptr<cv::imgrec::CNNImageRecognizer> _ptr = cv::imgrec::createGoogleNetRecognizer( cv::String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.prototxt"),
                                                                                        cv::String("C:/Programming/3rdParties/Caffe/models/bvlc_googlenet/bvlc_googlenet.caffemodel") );*/

    cv::Ptr<cv::imgrec::CNNImageRecognizer> _ptr = cv::imgrec::createDlibWhalesRecognizer(cv::String("C:/Testdata/Kaggle/Whales/whales_metric_network_renset.dat"));

    qInfo("Step_2 - Generating descriptions for labels");
    QStringList _lsubdirname = _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    const bool _visualize = _cmdparser.get<bool>("visualize");
    for(int i = 0; i < _lsubdirname.size(); ++i) {
        qInfo("   Label %d", i);
        QDir _subdir(_dir.absolutePath().append("/%1").arg(_lsubdirname.at(i)));
        QStringList _extensions;
        _extensions << "*.png" << "*.jpeg" << "*.bmp" << "*.jpg";
        QStringList _lfilename = _subdir.entryList(_extensions, QDir::Files | QDir::NoDotAndDotDot);

        for(int j = 0; j < _lfilename.size(); ++j) {
            QString _filename = _subdir.absoluteFilePath(_lfilename.at(j));
            std::vector<cv::Mat> _vmat;
            _vmat.push_back(cv::imread(_filename.toLocal8Bit().constData(), cv::IMREAD_UNCHANGED));
            std::vector<int> _vlbl;
            _vlbl.push_back(i);
            _ptr->update(_vmat,_vlbl,_visualize); // maybe it is not fastest way, but it is pretty interactive instead
        }
        if(_lfilename.size() > 0) {
            _ptr->setLabelInfo(i,_lsubdirname.at(i).toUtf8().constData());
        }
    }

    cv::String _outputfilename = _cmdparser.get<cv::String>("outputfile");
    qInfo("Saving descriptions in %s", _outputfilename.c_str());
    _ptr->ImageRecognizer::save(_outputfilename);
    return 0;
}
