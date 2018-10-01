#include "qoirtcli.h"

#include <QFile>
#include <QTimer>
#include <QDataStream>
#include <QJsonObject>
#include <QJsonDocument>

QOIRTCli::QOIRTCli(OIRTTask::TaskCode _taskcode, QObject *parent): QObject(parent),
    taskcode(_taskcode)
{
    connect(&tcpsocket,SIGNAL(connected()),this,SLOT(sendTask()));
    connect(&tcpsocket,SIGNAL(readyRead()),this,SLOT(readSocket()));
}

void QOIRTCli::connectTo(const QHostAddress &_addr, quint16 _port)
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

void QOIRTCli::deleteFiles()
{
    if(!imgfilename.isEmpty()) {
        bool _deleted = QFile::remove(imgfilename);
        qDebug("File %s delete status: %s", imgfilename.toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
    }
    if(!vimgfilename.isEmpty()) {
        bool _deleted = QFile::remove(vimgfilename);
        qDebug("File %s delete status: %s", vimgfilename.toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
    }
    if(!whitelistfilename.isEmpty()) {
        bool _deleted = QFile::remove(whitelistfilename);
        qDebug("File %s delete status: %s", whitelistfilename.toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
    }
    emit filesDeleted();
}

void QOIRTCli::sendTask()
{
    QDataStream _ods(&tcpsocket);
    _ods.setVersion(QDataStream::Qt_5_0);

    _ods << OIRTTask::getTaskCodeValue(taskcode);
    qDebug("taskcode: %u", (uint)OIRTTask::getTaskCodeValue(taskcode));

    switch(taskcode) {
        case OIRTTask::RememberLabel: {
            qDebug("RememberLabel");             
            _ods << static_cast<qint32>(labelinfo.size());
            _ods << labelinfo;
            QByteArray _encimg = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_encimg.size());
            _ods << _encimg;
        } break;

        case OIRTTask::DeleteLabel:
            qDebug("DeleteLabel");
            _ods << static_cast<qint32>(labelinfo.size());
            _ods << labelinfo;
            break;

        case OIRTTask::IdentifyImage: {
            qDebug("IdentifyImage");
            QByteArray _encimg = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_encimg.size());
            _ods << _encimg;
        } break;

        case OIRTTask::RecognizeImage: {
            qDebug("RecognizeImage");
            QByteArray _encimg = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_encimg.size());
            _ods << _encimg;
        } break;

        case OIRTTask::VerifyImage: {
            qDebug("VerifyImage");
            QByteArray _data = __readImgfileContent(imgfilename);
            _ods << static_cast<qint32>(_data.size());
            _ods << _data;
            _data = __readImgfileContent(vimgfilename);
            _ods << static_cast<qint32>(_data.size());
            _ods << _data;
        }

        case OIRTTask::UpdateWhitelist: {
            qDebug("UpdateWhitelist");
            QFile _file(whitelistfilename);
            _file.open(QIODevice::ReadOnly);
            QByteArray _json = _file.readAll();
            _ods << static_cast<qint32>(_json.size());
            _ods << _json;
        } break;

        default:
            qDebug("UnknownTask");
            // To suppress warnings
            break;
    }
}

void QOIRTCli::readSocket()
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
    _ids >> _repeat;
    qInfo("%s", QString(_repeat.constData()).toUtf8().constData());
    emit taskAccomplished();
}

QByteArray QOIRTCli::__readImgfileContent(const QString &_filename)
{
    QFile _tmpfile(_filename);
    if(_tmpfile.open(QFile::ReadOnly) == false) {
        qWarning("QOIRTCli::Warning - can not read %s",_filename.toUtf8().constData());
    }
    return _tmpfile.readAll();
}

void QOIRTCli::setWhitelistfilename(const QString &value)
{
    whitelistfilename = value;
}

void QOIRTCli::setVimgfilename(const QString &value)
{
    vimgfilename = value;
}

void QOIRTCli::setImgfilename(const QString &value)
{
    imgfilename = value;
}

void QOIRTCli::setLabelinfo(const QByteArray &value)
{
    labelinfo = value;
}
