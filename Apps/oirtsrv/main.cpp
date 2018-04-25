#include <QCoreApplication>
#include <QTimer>

#include "qoirtserver.h"
#include "qfacerecognizer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    //-----------------------------
    int port = 8080;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -p - port number to listen (default: %d)", port);
                return 0;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;


        }
    }

    QOIRTServer server;
    if(server.start(port) == false) {
        qWarning("Abort...");
        return 1;
    }

    QFaceRecognizer qfacerec;
    qfacerec.loadResources(a.applicationDirPath().append("/shape_predictor_5_face_landmarks.dat"),
                           a.applicationDirPath().append("/dlib_face_recognition_resnet_model_v1.dat"));
    qfacerec.loadLabels();



    return a.exec();
}
