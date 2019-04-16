#ifndef QOICTSERVER_H
#define QOICTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

struct OICTTask {

    OICTTask(QTcpSocket *_tcpsocket=nullptr) :
        tcpsocket(_tcpsocket),
        encimgbytes(-1),
        encimgaccepted(false) {}

    QTcpSocket *tcpsocket;
    QByteArray encimg;
    qint32     encimgbytes;
    bool       encimgaccepted;
};


class QOICTServer : public QObject
{
    Q_OBJECT
public:
    explicit QOICTServer(QObject *parent = 0);
    ~QOICTServer();

signals:
    void classifyImage(qintptr _taskid, const QByteArray &_encimg);

public slots:    
    bool start(const QString &_addr, quint16 _port);
    void stop();
    void repeatToClient(qintptr _taskid, const QByteArray &_repeat);   

private slots:
    void handleError(QAbstractSocket::SocketError);
    void getNewTask();
    void readClient();
    void removeClient();

private:
    QTcpServer tcpserver;
    QMap<qintptr, OICTTask> tasks;
};

#endif // QOICTSERVER_H
