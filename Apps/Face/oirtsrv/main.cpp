#include <QCoreApplication>
#include <QFileInfo>
#include <QThread>
#include <QTimer>

#include "qoirtserver.h"
#include "qrecognizer.h"

#include "dlibfacerecognizer.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    QCoreApplication a(argc, argv);
    //-----------------------------
    quint16 port = 8080;
    QString address = "127.0.0.1";
    QString labelsfilename = a.applicationDirPath().append("/labels.yml");
    double recthresh = 0.485;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -a[str]  - addres to listen (default: %s)", address.toUtf8().constData());
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
    QRecognizer qfacerec;
    qfacerec.loadResources(cv::oirt::createDlibFaceRecognizer(a.applicationDirPath().append("/shape_predictor_5_face_landmarks.dat").toUtf8().constData(),
                                                              a.applicationDirPath().append("/dlib_face_recognition_resnet_model_v1.dat").toUtf8().constData(),
                                                              cv::oirt::DistanceType::Euclidean,
                                                              recthresh));
    qfacerec.setLabelsfilename(labelsfilename);
    QFileInfo _fi(labelsfilename);
    if(_fi.exists())
        qfacerec.loadLabels(labelsfilename);
    QThread qfacerecthread;
    qfacerec.moveToThread(&qfacerecthread);

    QObject::connect(&server,SIGNAL(rememberLabel(qintptr,QByteArray,QByteArray)),&qfacerec,SLOT(rememberLabel(qintptr,QByteArray,QByteArray)));
    QObject::connect(&server,SIGNAL(identifyImage(qintptr,QByteArray)),&qfacerec,SLOT(identifyImage(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(recognizeImage(qintptr,QByteArray)),&qfacerec,SLOT(recognizeImage(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(deleteLabel(qintptr,QByteArray)),&qfacerec,SLOT(deleteLabel(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(askLabelsList(qintptr)),&qfacerec,SLOT(getLabelsList(qintptr)));
    QObject::connect(&server,SIGNAL(verifyImage(qintptr,QByteArray,QByteArray)),&qfacerec,SLOT(verifyImage(qintptr,QByteArray,QByteArray)));
    QObject::connect(&server,SIGNAL(updateWhitelist(qintptr,QByteArray)),&qfacerec,SLOT(updateWhitelist(qintptr,QByteArray)));
    QObject::connect(&server,SIGNAL(dropWhitelist(qintptr)),&qfacerec,SLOT(dropWhitelist(qintptr)));
    QObject::connect(&qfacerec,SIGNAL(taskAccomplished(qintptr,QByteArray)),&server,SLOT(repeatToClient(qintptr,QByteArray)));
    QObject::connect(&qfacerecthread,SIGNAL(started()),&qfacerec,SLOT(initBackupTimer()));

    qfacerecthread.start();

    return a.exec();
}
