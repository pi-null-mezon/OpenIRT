#ifndef QOIRTCLI_H
#define QOIRTCLI_H

#include <QObject>

#include <QTcpSocket>
#include <QHostAddress>

#include "oirttask.h"

class QOIRTCli : public QObject
{
    Q_OBJECT
public:
    QOIRTCli(OIRTTask::TaskCode _taskcode, QObject *parent = 0);

    void connectTo(const QHostAddress &_addr, quint16 _port);

    QByteArray getLabelinfo() const;
    void setLabelinfo(const QByteArray &value);

    QByteArray *getEncimg() const;
    void setEncimg(QByteArray *value);

signals:
    void taskAccomplished();

private slots:
    void sendTask();
    void readSocket();

private:
    QTcpSocket tcpsocket;

    OIRTTask::TaskCode  taskcode;
    QByteArray          labelinfo;
    QByteArray          *encimg;

    int repeatlength;
};

#endif // QOIRTCLI_H
