#include "qoirtcli.h"

#include <QFile>
#include <QTimer>
#include <QDataStream>

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
    QTimer::singleShot(10000,[=]() {qWarning("Seems that %s:%u can not be accessed",_addr.toString().toUtf8().constData(),static_cast<uint>(_port));});
}

void QOIRTCli::deleteAllFiles()
{
    bool _deleted = QFile::remove(getImgfilename());
    qDebug("File %s delete status: %s", getImgfilename().toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
    if(taskcode == OIRTTask::TaskCode::VerifyImage) {
        _deleted = QFile::remove(getVimgfilename());
        qDebug("File %s delete status: %s", getVimgfilename().toUtf8().constData(), _deleted ? "deleted" : "can not be deleted");
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
            QByteArray _encimg = __readImgfileContent(getImgfilename());
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
            QByteArray _encimg = __readImgfileContent(getImgfilename());
            _ods << static_cast<qint32>(_encimg.size());
            _ods << _encimg;
        } break;

        case OIRTTask::VerifyImage: {
            qDebug("VerifyImage");
            QByteArray _data = __readImgfileContent(getImgfilename());
            _ods << static_cast<qint32>(_data.size());
            _ods << _data;
            _data = __readImgfileContent(getVimgfilename());
            _ods << static_cast<qint32>(_data.size());
            _ods << _data;
        }

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

QString QOIRTCli::getVimgfilename() const
{
    return vimgfilename;
}

void QOIRTCli::setVimgfilename(const QString &value)
{
    vimgfilename = value;
}

QString QOIRTCli::getImgfilename() const
{
    return imgfilename;
}

void QOIRTCli::setImgfilename(const QString &value)
{
    imgfilename = value;
}

QByteArray QOIRTCli::getLabelinfo() const
{
    return labelinfo;
}

void QOIRTCli::setLabelinfo(const QByteArray &value)
{
    labelinfo = value;
}
