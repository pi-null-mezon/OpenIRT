#include <QCoreApplication>
#include <QDataStream>
#include <QHostAddress>
#include <QTcpSocket>
#include <QFile>

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
      fprintf(stdout, "%s", localMsg.constData());
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
    QString imgfilename;
    QString labelinfo;
    OIRTTask::TaskCode taskcode = OIRTTask::UnknownTask;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n", APP_NAME, APP_VERSION);
                qInfo(" -p[int] - port number to connect (default: %u)", (uint)port);
                qInfo(" -i[str] - filename of the image that should be processed");
                qInfo(" -l[str] - label info string");
                qInfo(" -t[int] - task code {RememberLabel=1, DeleteLabel=2, IdentifyImage=3}");
                return 0;

            case 'i':
                imgfilename = QString::fromLocal8Bit(++argv[0]);
                break;

            case 'p':
                port = QString(++argv[0]).toInt();
                break;

            case 'l':
                labelinfo = QString::fromLocal8Bit(++argv[0]);
                break;

            case 't': {
                    quint8 _val = static_cast<quint8>(QString(++argv[0]).toUInt());
                    taskcode = OIRTTask::getTaskCode(_val);
                } break;
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

    QByteArray encimg;
    if(taskcode == OIRTTask::IdentifyImage || taskcode == OIRTTask::RememberLabel) {
        if(imgfilename.isEmpty()) {
            qWarning("Empty input file name! Abort...");
            return 3;
        }
        QFile _file(imgfilename);
        if(_file.open(QFile::ReadOnly) == false) {
            qWarning("Can not open file %s! Abort...",imgfilename.toUtf8().constData());
            return 4;
        } else {
            encimg = qMove(_file.readAll());
        }
    }

    QOIRTCli _client(taskcode);
    _client.setLabelinfo(labelinfo.toUtf8());
    _client.setEncimg(&encimg);
    _client.connectTo(QHostAddress::LocalHost,port);

    QObject::connect(&_client,&QOIRTCli::taskAccomplished,&a,&QCoreApplication::quit);

    return a.exec();
}
