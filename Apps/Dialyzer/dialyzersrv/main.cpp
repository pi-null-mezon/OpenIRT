#include <QCoreApplication>
#include <QFileInfo>
#include <QThread>
#include <QTimer>

#include "qoirtserver.h"
#include "qrecognizer.h"

#include "dialyzerrecognizer.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    QCoreApplication a(argc, argv);
    //-----------------------------
    quint16 port = 8081;
    QString address = "127.0.0.1";
    QString labelsfilename = a.applicationDirPath().append("/labels.yml");
    double  recthresh = 0.40;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -a[str]  - address to listen (default: %s)", address.toUtf8().constData());
                qInfo(" -p[int]  - port number to listen (default: %u)", (uint)port);
                qInfo(" -l[str]  - filename of the file to store labels (default: %s)", labelsfilename.toUtf8().constData());
                qInfo(" -t[real] - recognition threshold (default: %f)", recthresh);
                return 0;

            case 'l':
                labelsfilename = QString(++argv[0]);
                break;

            case 'a':
                address = QString(++argv[0]);
                break;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;

            case 't':
                recthresh = QString(++argv[0]).toDouble();
                break;
        }
    }

    // Let's run server
    QOIRTServer server;
    if(server.start(address,port) == false) {
        qWarning("Abort...");
        return 1;
    }

    // Let's run facerecognizer in separate thread
    QRecognizer qrecognizer;
    qrecognizer.loadResources(cv::oirt::createDialyzerRecognizer(a.applicationDirPath().append("/dlib_resnet_metric_dialyzer.dat").toUtf8().constData(),
                                                                 cv::oirt::DistanceType::Euclidean,
                                                                 recthresh));
    qrecognizer.setLabelsfilename(labelsfilename);
    QFileInfo _fi(labelsfilename);
    if(_fi.exists())
        qrecognizer.loadLabels(labelsfilename);
    QThread qrecognizerthread;
    qrecognizer.moveToThread(&qrecognizerthread);

    QObject::connect(&server,SIGNAL(rememberLabel(qintptr,QByteArray,QByteArray)),&qrecognizer,SLOT(rememberLabel(qintptr,QByteArray,QByteArray)));
    QObject::connect(&server,SIGNAL(identifyImage(qintptr,QByteArray)),&qrecognizer,SLOT(identifyImage(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(deleteLabel(qintptr,QByteArray)),&qrecognizer,SLOT(deleteLabel(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(askLabelsList(qintptr)),&qrecognizer,SLOT(getLabelsList(qintptr)));
    QObject::connect(&server,SIGNAL(verifyImage(qintptr,QByteArray,QByteArray)),&qrecognizer,SLOT(verifyImage(qintptr,QByteArray,QByteArray)));
    QObject::connect(&server,SIGNAL(updateWhitelist(qintptr,QByteArray)),&qrecognizer,SLOT(updateWhitelist(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(dropWhitelist(qintptr)),&qrecognizer,SLOT(dropWhitelist(qintptr)));
    QObject::connect(&server,SIGNAL(recognizeImage(qintptr,QByteArray)),&qrecognizer,SLOT(recognizeImage(qintptr,QByteArray)));
    QObject::connect(&qrecognizer,SIGNAL(taskAccomplished(qintptr,QByteArray)),&server,SLOT(repeatToClient(qintptr,QByteArray)));
    QObject::connect(&qrecognizerthread,SIGNAL(started()),&qrecognizer,SLOT(initBackupTimer()));

    qrecognizerthread.start();

    return a.exec();
}
