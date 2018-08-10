#include <QCoreApplication>
#include <QDataStream>
#include <QHostAddress>
#include <QTcpSocket>
#include <QFileInfo>

#include "qoirtcli.h"

void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  QByteArray localMsg = msg.toLocal8Bit();
  switch (type) {
  case QtDebugMsg:
      fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
  case QtInfoMsg:
      fprintf(stdout, "%s", localMsg.constData());
      //fprintf(stdout, "Info: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
  case QtWarningMsg:
      fprintf(stdout, "%s\n", localMsg.constData());
      //fprintf(stdout, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
  case QtCriticalMsg:
      fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      break;
  case QtFatalMsg:
      fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
      abort();
  }
}

int main(int argc, char *argv[])
{
#ifdef Q_OS_WIN
    setlocale(LC_CTYPE,"Rus");
#endif
    qInstallMessageHandler(myMessageOutput);
    QCoreApplication a(argc, argv);
    //-----------------------------
    quint16 port = 8080;
    QHostAddress srvaddr = QHostAddress::LocalHost;
    QString imgfilename;
    QString labelinfo;
    OIRTTask::TaskCode taskcode = OIRTTask::UnknownTask;
    bool deletefile = false;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n\n", APP_NAME, APP_VERSION);
                qInfo(" -a[str] - address of the server to connect (default: localhost i.e 127.0.0.1)\n");
                qInfo(" -p[int] - port of the server to connect (default: 8080)\n");
                qInfo(" -i[str] - filename of the image that should be processed\n");
                qInfo(" -l[str] - label info string\n");
                qInfo(" -t[int] - task code {RememberLabel=1, DeleteLabel=2, IdentifyImage=3, AskLabels=4}\n");
                qInfo(" -d      - delete file after server repeat\n");
                return 0;

            case 'a':
                srvaddr = QHostAddress(QString(++argv[0]));
                break;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;

            case 'i':
                imgfilename = QString::fromLocal8Bit(++argv[0]);
                break;

            case 'l':
                labelinfo = QString::fromLocal8Bit(++argv[0]);
                break;

            case 't': {
                    quint8 _val = static_cast<quint8>(QString(++argv[0]).toUInt());
                    taskcode = OIRTTask::getTaskCode(_val);
                } break;

            case 'd':
                deletefile = true;
                break;
        }
    }   
    if(taskcode == OIRTTask::UnknownTask) {
        qWarning("You have not specify task! Abort...");
        return 1;
    }

    if((taskcode == OIRTTask::DeleteLabel || taskcode == OIRTTask::RememberLabel) && labelinfo.isEmpty()) {
        qWarning("You have not specify labelinfo! Abort...");
        return 2;
    }

    if(taskcode == OIRTTask::IdentifyImage || taskcode == OIRTTask::RememberLabel) {
        if(imgfilename.isEmpty()) {
            qWarning("Empty input file name! Abort...");
            return 3;
        }
        QFileInfo _finfo(imgfilename);
        if(_finfo.exists() == false) {
            qWarning("Input file can not be found! Abort...");
            return 4;
        }
    }

    QOIRTCli _client(taskcode);
    _client.setLabelinfo(labelinfo.toUtf8());
    _client.setImgfilename(imgfilename);
    _client.connectTo(srvaddr,port);

    if(deletefile == false) {
        QObject::connect(&_client,&QOIRTCli::taskAccomplished,&a,&QCoreApplication::quit);
    } else {
        QObject::connect(&_client,&QOIRTCli::taskAccomplished,&_client,&QOIRTCli::deleteAllFiles);
        QObject::connect(&_client,&QOIRTCli::filesDeleted,&a,&QCoreApplication::quit);
    }

    return a.exec();
}
