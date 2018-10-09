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
    QRecognizer qrecognizer;
    qrecognizer.loadResources(cv::oirt::createDialyzerRecognizer(a.applicationDirPath().append("/dlib_resnet_metric_dialyzer.dat").toUtf8().constData()));
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
