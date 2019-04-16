#include <QCoreApplication>
#include <QFileInfo>
#include <QThread>

#include "qoictserver.h"
#include "qclassifier.h"

#include "replayattackdetector.h"

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    QCoreApplication a(argc, argv);
    //-----------------------------
    quint16 port = 8080;
    QString address = "127.0.0.1";
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -a[str]  - addres to listen (default: %s)", address.toUtf8().constData());
                qInfo(" -p[int]  - port number to listen (default: %u)", (uint)port);                
                return 0;

            case 'a':
                address = QString(++argv[0]);
                break;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;           
        }
    }

    // Let's check resources
    QFileInfo _finfo(a.applicationDirPath().append("/replay_attack_net_v5.dat"));
    if(!_finfo.exists()) {
        qWarning("Can not open '%s'! Abort...",_finfo.fileName().toUtf8().constData());
        return 1;
    }
    _finfo = QFileInfo(a.applicationDirPath().append("/shape_predictor_5_face_landmarks.dat"));
    if(!_finfo.exists()) {
        qWarning("Can not open '%s'! Abort...",_finfo.fileName().toUtf8().constData());
        return 2;
    }
    // Let's run server
    QOICTServer server;
    if(server.start(address,port) == false) {
        qWarning("Can not listen! Abort...");
        return 3;
    }

    // Let's run facerecognizer in separate thread
    QClassifier qclassifier;
    qclassifier.loadResources(cv::oirt::ReplayAttackDetector::createReplayAttackDetector(a.applicationDirPath().append("/replay_attack_net_v5.dat").toUtf8().constData(),
                                                                                         a.applicationDirPath().append("/shape_predictor_5_face_landmarks.dat").toUtf8().constData()));

    QThread qclassifierthread;
    qclassifier.moveToThread(&qclassifierthread);

    QObject::connect(&server,SIGNAL(classifyImage(qintptr,QByteArray)),&qclassifier,SLOT(classifyImage(qintptr,QByteArray)));
    QObject::connect(&qclassifier,SIGNAL(taskAccomplished(qintptr,QByteArray)),&server,SLOT(repeatToClient(qintptr,QByteArray)));

    qclassifierthread.start();

    return a.exec();
}
