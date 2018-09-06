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

    void setLabelinfo(const QByteArray &value);
    void setImgfilename(const QString &value);
    void setVimgfilename(const QString &value);
    void setWhitelistfilename(const QString &value);

signals:
    void taskAccomplished();
    void filesDeleted();

public slots:
    void deleteFiles();

private slots:
    void sendTask();
    void readSocket();

private:
    QByteArray __readImgfileContent(const QString &_filename);
    QTcpSocket tcpsocket;

    OIRTTask::TaskCode  taskcode;
    QByteArray          labelinfo;
    QString             imgfilename;
    QString             vimgfilename;
    QString             whitelistfilename;

    int                 repeatlength;
};

#endif // QOIRTCLI_H
