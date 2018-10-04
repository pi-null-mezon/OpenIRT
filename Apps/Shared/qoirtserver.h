#ifndef QOIRTSERVER_H
#define QOIRTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMap>

#include "oirttask.h"

class QOIRTServer : public QObject
{
    Q_OBJECT
public:
    explicit QOIRTServer(QObject *parent = 0);
    ~QOIRTServer();
signals:
    void rememberLabel(qintptr _taskid, const QByteArray &_labelinfo, const QByteArray &_encimg);
    void deleteLabel(qintptr _taskid, const QByteArray &_labelinfo);
    void identifyImage(qintptr _taskid, const QByteArray &_encimg);
    void recognizeImage(qintptr _taskid, const QByteArray &_encimg);
    void verifyImage(qintptr _taskid, const QByteArray &_eimg, const QByteArray &_vimg);
    void askLabelsList(qintptr _taskid);
    void updateWhitelist(qintptr _taskid, const QByteArray &_jsonwhitelist);
    void dropWhitelist(qintptr _taskid);

public slots:    
    bool start(quint16 _port);
    void stop();
    void repeatToClient(qintptr _taskid, const QByteArray &_repeat);   

private slots:
    void handleError(QAbstractSocket::SocketError);
    void getNewTask();
    void readClient();
    void removeClient();

private:
    QTcpServer tcpserver;
    QMap<qintptr, OIRTTask> tasks;
};

#endif // QOIRTSERVER_H
