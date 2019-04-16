#ifndef QOICTCLI_H
#define QOICTCLI_H

#include <QObject>

#include <QTcpSocket>
#include <QHostAddress>

class QOICTCli : public QObject
{
    Q_OBJECT
public:
    QOICTCli(QObject *parent = 0);

    void connectTo(const QHostAddress &_addr, quint16 _port);
    void setImgfilename(const QString &value);

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
    QString imgfilename;
    int     repeatlength;
};

#endif // QOICTCLI_H
