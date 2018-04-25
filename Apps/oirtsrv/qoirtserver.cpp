#include "qoirtserver.h"

#include <QNetworkInterface>
#include <QHostAddress>
#include <QDateTime>

#include <QDataStream>

#include <QDebug>

QOIRTServer::QOIRTServer(QObject *parent) : QObject(parent)
{
    connect(&tcpserver, SIGNAL(newConnection()), this, SLOT(getNewTask()));
}

QOIRTServer::~QOIRTServer()
{
    stop();
}

bool QOIRTServer::start(int _port)
{
    if(tcpserver.isListening()) {
        stop();
    }
    if(tcpserver.listen(QHostAddress::LocalHost, _port)) {
        qInfo("QOIRTServer: Start to listening localhost:%d", _port);
        /*QList<QHostAddress> _laddr = QNetworkInterface::allAddresses();
        if(_laddr.size() > 0) {
            int k = 1;
            for(int i = 0; i < _laddr.size(); ++i) {
                if(_laddr.at(i).toIPv4Address()) {
                    qInfo("%d) %s:%d", k++, _laddr.at(i).toString().toUtf8().constData(), _port);
                }
            }
        } else {
            qWarning("QMAGTServer: server can not listen port %d!",_port);
        }*/
        return true;
    } else {
        qWarning("QOIRTServer: %s", tcpserver.errorString().toUtf8().constData());
    }
    return false;
}

void QOIRTServer::getNewTask()
{
    QTcpSocket* _clientSocket = tcpserver.nextPendingConnection();
    qintptr _taskid = _clientSocket->socketDescriptor();
    qDebug("QOIRTServer: New connection %d has been established", static_cast<int>(_taskid));
    connect(_clientSocket,SIGNAL(readyRead()),this, SLOT(readClient()));
    tasks[_taskid] = OIRTTask(_clientSocket);
}


void QOIRTServer::readClient()
{
    qintptr _taskid = ((QTcpSocket*)sender())->socketDescriptor();
    OIRTTask *_task = &tasks[_taskid];

    QDataStream _ids(_task->tcpsocket);
    _ids.setVersion(QDataStream::Qt_5_0);

    if(_task->taskcode == OIRTTask::UnknownTask) {
        if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(quint8))
            return;
         quint8 _code;
         _ids >> _code;
         _task->taskcode = OIRTTask::getTaskCode(_code);
    }

    switch(_task->taskcode) {
        case OIRTTask::RememberLabel:
            // WAIT LABEL INFO
            if(_task->labelinfobytes == -1) {
                if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                    return;
                _ids >> _task->labelinfobytes;
            }
            if(_task->labelaccepted == false) {
                if(_task->tcpsocket->bytesAvailable() < _task->labelinfobytes)
                    return;
                _task->labeinfo.resize(_task->labelinfobytes);
                _ids.readRawData(_task->labeinfo.data(), _task->labelinfobytes);
                _task->labelaccepted = true;
            }
            // WAIT ENCODED PICTURE
            if(_task->encimgbytes == -1) {
                if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                    return;
                _ids >> _task->encimgbytes;
            }
            if(_task->encimgaccepted == false) {
                if(_task->tcpsocket->bytesAvailable() < _task->encimgbytes)
                    return;
                _task->encimg.resize(_task->labelinfobytes);
                _ids.readRawData(_task->encimg.data(), _task->encimgbytes);
                _task->encimgaccepted = true;
            }
            break;

        case OIRTTask::DeleteLabel:
            // WAIT LABEL INFO
            if(_task->labelinfobytes == -1) {
                if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                    return;
                _ids >> _task->labelinfobytes;
            }
            if(_task->labelaccepted == false) {
                if(_task->tcpsocket->bytesAvailable() < _task->labelinfobytes)
                    return;
                _task->labeinfo.resize(_task->labelinfobytes);
                _ids.readRawData(_task->labeinfo.data(), _task->labelinfobytes);
                _task->labelaccepted = true;
            }
            break;

        case OIRTTask::IdentifyImage:
            // WAIT ENCODED PICTURE
            if(_task->encimgbytes == -1) {
                if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                    return;
                _ids >> _task->encimgbytes;
            }
            if(_task->encimgaccepted == false) {
                if(_task->tcpsocket->bytesAvailable() < _task->encimgbytes)
                    return;
                _task->encimg.resize(_task->labelinfobytes);
                _ids.readRawData(_task->encimg.data(), _task->encimgbytes);
                _task->encimgaccepted = true;
            }
            break;
    }
}

void QOIRTServer::repeatToClient(qintptr _taskid, const QByteArray &_repeat)
{
    OIRTTask *_task = &tasks[_taskid];
    QDataStream _ods(_task->tcpsocket);
    _ods.setVersion(QDataStream::Qt_5_0);

    _ods << _repeat.size();
    _ods << _repeat;

    qDebug("QOIRTServer: Client connection %d will be closed", static_cast<int>(_taskid));
    tasks[_taskid].tcpsocket->close();
    tasks.remove(_taskid);
}

void QOIRTServer::stop()
{
    if(tcpserver.isListening()) {
        qDebug("QOIRTServer: Stop");
        foreach(qintptr i, tasks.keys()) {
            tasks[i].tcpsocket->close();
            tasks.remove(i);
        }
        tcpserver.close();
    }
}
