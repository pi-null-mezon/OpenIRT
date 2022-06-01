#include "qoictserver.h"

#include <QAbstractSocket>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDateTime>
#include <QDataStream>

QOICTServer::QOICTServer(QObject *parent) : QObject(parent)
{
    qRegisterMetaType<qintptr>("qintptr");
    connect(&tcpserver, SIGNAL(newConnection()), this, SLOT(getNewTask()));
}

QOICTServer::~QOICTServer()
{
    stop();
}

bool QOICTServer::start(const QString &_addr, quint16 _port)
{
    if(tcpserver.isListening()) {
        stop();
    }
    if(tcpserver.listen(QHostAddress(_addr), _port)) {
        qInfo("Server starts listening on %s:%u",_addr.toUtf8().constData(),(uint)_port);
        return true;
    } else {
        qWarning("QOICTServer: %s", tcpserver.errorString().toUtf8().constData());
    }
    return false;
}

void QOICTServer::getNewTask()
{
    QTcpSocket* _clientSocket = tcpserver.nextPendingConnection();
    qintptr _taskid = _clientSocket->socketDescriptor();
    qDebug("QOICTServer: new connection %d has been established",static_cast<int>(_taskid));
    connect(_clientSocket,SIGNAL(readyRead()),this,SLOT(readClient()));
    connect(_clientSocket,SIGNAL(disconnected()),this,SLOT(removeClient()));
    connect(_clientSocket,SIGNAL(error(QAbstractSocket::SocketError)),this, SLOT(handleError(QAbstractSocket::SocketError)));
    tasks[_taskid] = OICTTask(_clientSocket);
}

void QOICTServer::readClient()
{
    qintptr _taskid = qobject_cast<QTcpSocket*>(sender())->socketDescriptor();
    if(tasks.contains(_taskid)) {

        OICTTask *_task = &tasks[_taskid];
        QDataStream _ids(_task->tcpsocket);
        _ids.setVersion(QDataStream::Qt_5_0);

        if(_task->taskcode == OICTTask::UnknownTask) {
            if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(quint8))
                return;
            quint8 _code;
            _ids >> _code;
            _task->taskcode = OICTTask::getTaskCode(_code);
            qDebug("Task code recieved: %d", _task->taskcode);
        }

        switch(_task->taskcode) {
            case OICTTask::AskLabelsList:
                emit listLabels(_taskid);
                break;

            case OICTTask::Classify:
                if(_task->encimgbytes == -1) {
                    if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                        return;
                    _ids >> _task->encimgbytes;
                }
                if(_task->encimgaccepted == false) {
                    if(_task->tcpsocket->bytesAvailable() < _task->encimgbytes)
                        return;
                    _task->encimg.resize(_task->encimgbytes);
                    _ids.readRawData(_task->encimg.data(),_task->encimg.size());
                    _task->encimgaccepted = true;
                    emit classify(_taskid,_task->encimg);
                }
                break;

            case OICTTask::Predict:
                if(_task->encimgbytes == -1) {
                    if(_task->tcpsocket->bytesAvailable() < (qint64)sizeof(qint32))
                        return;
                    _ids >> _task->encimgbytes;
                }
                if(_task->encimgaccepted == false) {
                    if(_task->tcpsocket->bytesAvailable() < _task->encimgbytes)
                        return;
                    _task->encimg.resize(_task->encimgbytes);
                    _ids.readRawData(_task->encimg.data(),_task->encimg.size());
                    _task->encimgaccepted = true;
                    emit predict(_taskid,_task->encimg);
                }
                break;

            default:
                // TO SUPRESS WARNINGS
                break;
        }
    }
}

void QOICTServer::removeClient()
{
    QTcpSocket *_tcpsocket = qobject_cast<QTcpSocket*>(sender());
    _tcpsocket->deleteLater();
}

void QOICTServer::repeatToClient(qintptr _taskid, const QByteArray &_repeat)
{    
    if(tasks.contains(_taskid)) {
        OICTTask *_task = &tasks[_taskid];
        QDataStream _ods(_task->tcpsocket);
        _ods.setVersion(QDataStream::Qt_5_0);

        qDebug("%s",_repeat.constData());

        _ods << static_cast<qint32>(_repeat.size());
        _ods.writeRawData(_repeat.constData(),_repeat.size());

        qDebug("QOICTServer: connection %d will be closed", static_cast<int>(_taskid));
        _task->tcpsocket->close();
        tasks.remove(_taskid);
    }
}

void QOICTServer::stop()
{
    if(tcpserver.isListening()) {
        qDebug("QOICTServer: Stop");
        foreach(qintptr i, tasks.keys()) {
            tasks[i].tcpsocket->close();
            tasks.remove(i);
        }
        tcpserver.close();
    }
}

void QOICTServer::handleError(QAbstractSocket::SocketError _error)
{
    QTcpSocket* _tcpsocket = qobject_cast<QTcpSocket*>(sender());
    qintptr _taskid = _tcpsocket->socketDescriptor();
    qDebug("QOICTServer: error occured with connection %d - code %d \"%s\"",
           static_cast<int>(_taskid),
           _error,
           _tcpsocket->errorString().toUtf8().constData());
    tasks.remove(_taskid);
}
