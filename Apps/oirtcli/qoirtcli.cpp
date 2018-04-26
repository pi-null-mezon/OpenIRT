#include "qoirtcli.h"

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
}

void QOIRTCli::sendTask()
{
    QDataStream _ods(&tcpsocket);
    _ods.setVersion(QDataStream::Qt_5_0);

    _ods << OIRTTask::getTaskCodeValue(taskcode);
    qInfo("taskcode: %u", (uint)OIRTTask::getTaskCodeValue(taskcode));

    switch(taskcode) {
        case OIRTTask::RememberLabel:
            qDebug("RememberLabel");
            _ods << static_cast<qint32>(labelinfo.size());
            _ods << labelinfo;
            _ods << static_cast<qint32>(encimg->size());
            _ods << *encimg;
            break;

        case OIRTTask::DeleteLabel:
            qDebug("DeleteLabel");
            _ods << static_cast<qint32>(labelinfo.size());
            _ods << labelinfo;
            break;

        case OIRTTask::IdentifyImage:
            qDebug("IdentifyImage");
            _ods << static_cast<qint32>(encimg->size());
            _ods << *encimg;
            break;

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
    qDebug("Server repeat: %s", QString(_repeat.constData()).toUtf8().constData());
    emit taskAccomplished();
}

QByteArray *QOIRTCli::getEncimg() const
{
    return encimg;
}

void QOIRTCli::setEncimg(QByteArray *value)
{
    encimg = value;
}

QByteArray QOIRTCli::getLabelinfo() const
{
    return labelinfo;
}

void QOIRTCli::setLabelinfo(const QByteArray &value)
{
    labelinfo = value;
}
