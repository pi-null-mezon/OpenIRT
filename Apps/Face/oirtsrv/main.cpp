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
    QString labelsfilename = a.applicationDirPath().append("/labels.yml");
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -p - port number to listen (default: %u)", (uint)port);
                qInfo(" -l - filename of the file to store labels (default: %s)", labelsfilename.toUtf8().constData());
                return 0;

            case 'l':
                labelsfilename = QString(++argv[0]);
                break;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;
        }
    }

    // Let's run server
    QOIRTServer server;
    if(server.start(port) == false) {
        qWarning("Abort...");
        return 1;
    }

    // Let's run facerecognizer in separate thread
    QRecognizer qfacerec;
    qfacerec.loadResources(cv::oirt::createDlibFaceRecognizer(a.applicationDirPath().append("/shape_predictor_5_face_landmarks.dat").toUtf8().constData(),
                                                              a.applicationDirPath().append("/dlib_face_recognition_resnet_model_v1.dat").toUtf8().constData()));
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
