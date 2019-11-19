#include "qoictcli.h"

#include <QFile>
#include <QTimer>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

QOICTCli::QOICTCli(OICTTask::TaskCode _taskcode, QObject *parent): QObject(parent),
    taskcode(_taskcode)
{
    connect(&tcpsocket,SIGNAL(connected()),this,SLOT(sendTask()));
    connect(&tcpsocket,SIGNAL(readyRead()),this,SLOT(readSocket()));
}

void QOICTCli::connectTo(const QHostAddress &_addr, quint16 _port)
{
    tcpsocket.connectToHost(_addr,_port);
    repeatlength = -1;
    QTimer::singleShot(10000,[=]() {
                                        QJsonObject _json;
                                        _json["status"]  = "Error";
                                        _json["message"] = "Recognition server can not be accessed! It could be busy or unreachable...";
                                        qInfo("%s",QJsonDocument(_json).toJson().constData());
                                        emit taskAccomplished();
                                    });
}

void QOICTCli::deleteFiles()
{
    if(!imgfilename.isEmpty()) {
        bool _deleted = QFile::remove(imgfilename);
        qDebug("File %s delete status: %s", imgfilename.toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
    }    
    emit filesDeleted();
}

void QOICTCli::sendTask()
{
    QDataStream _ods(&tcpsocket);
    _ods.setVersion(QDataStream::Qt_5_0);

    _ods << OICTTask::getTaskCodeValue(taskcode);
    qDebug("taskcode: %u", (uint)OICTTask::getTaskCodeValue(taskcode));

    switch(taskcode) {
        case OICTTask::Classify: {
            qDebug("Classify");
            QByteArray _encimg = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_encimg.size());
            _ods.writeRawData(_encimg.constData(),_encimg.size());
        } break;

        case OICTTask::Predict: {
            qDebug("Classify");
            QByteArray _encimg = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_encimg.size());
            _ods.writeRawData(_encimg.constData(),_encimg.size());
        } break;

        default:
            // To supress warnings
            break;
    }
}

void QOICTCli::readSocket()
{
    QDataStream _ids(&tcpsocket);
    _ids.setVersion(QDataStream::Qt_5_0);

    if(repeatlength == -1) {
        if(tcpsocket.bytesAvailable() < (qint64)sizeof(qint32))
            return;
        _ids >> repeatlength;
    }
    if(tcpsocket.bytesAvailable() < repeatlength) {
        return;
    }
    QByteArray _repeat;
    _repeat.resize(repeatlength);
    _ids.readRawData(_repeat.data(),_repeat.size());
    qInfo("%s", QString(_repeat.constData()).toUtf8().constData());
    emit taskAccomplished();
}

QByteArray QOICTCli::__readImgfileContent(const QString &_filename)
{
    QFile _tmpfile(_filename);
    if(_tmpfile.open(QFile::ReadOnly) == false) {
        qWarning("QOICTCli::Warning - can not read %s",_filename.toUtf8().constData());
    }
    return _tmpfile.readAll();
}

void QOICTCli::setImgfilename(const QString &value)
{
    imgfilename = value;
}
