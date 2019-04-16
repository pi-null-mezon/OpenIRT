#include <QCoreApplication>
#include <QDataStream>
#include <QHostAddress>
#include <QTcpSocket>
#include <QFileInfo>

#include "qoictcli.h"

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
      fprintf(stderr, "%s\n", localMsg.constData());
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
    bool deletefile = false;
    while((--argc > 0) && ((*++argv)[0] == '-')) {
        switch(*++argv[0]) {
            case 'h':
                qInfo("%s v.%s\n\n", APP_NAME, APP_VERSION);
                qInfo(" -a[str] - address of the server to connect (default: localhost i.e 127.0.0.1)\n");
                qInfo(" -p[int] - port of the server to connect (default: %u)\n",static_cast<uint>(port));
                qInfo(" -i[str] - filename of the image that should be processed\n");               
                qInfo(" -d      - delete image files after server's repeat\n");
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

            case 'd':
                deletefile = true;
                break;
        }
    }   

    QFileInfo _finfo(imgfilename);
    if(_finfo.exists() == false) {
        qWarning("Input file '%s' can not be found! Abort...",_finfo.absoluteFilePath().toUtf8().constData());
        return 1;
    }

    QOICTCli _client;
    _client.setImgfilename(imgfilename);
    _client.connectTo(srvaddr,port);

    if(deletefile == false) {
        QObject::connect(&_client,&QOICTCli::taskAccomplished,&a,&QCoreApplication::quit);
    } else {
        QObject::connect(&_client,&QOICTCli::taskAccomplished,&_client,&QOICTCli::deleteFiles);
        QObject::connect(&_client,&QOICTCli::filesDeleted,&a,&QCoreApplication::quit);
    }

    return a.exec();
}
