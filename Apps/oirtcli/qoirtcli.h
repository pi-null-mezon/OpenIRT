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

    QString getImgfilename() const;
    void setImgfilename(const QString &value);

signals:
    void taskAccomplished();
    void filesDeleted();

public slots:
    void deleteAllFiles();

private slots:
    void sendTask();
    void readSocket();

private:
    QByteArray __readImgfileContent();
    QTcpSocket tcpsocket;

    OIRTTask::TaskCode  taskcode;
    QByteArray          labelinfo;
    QString             imgfilename;

    int repeatlength;
};

#endif // QOIRTCLI_H
