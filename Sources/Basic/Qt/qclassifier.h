#ifndef QCLASSIFIER_H
#define QCLASSIFIER_H

#include <QObject>
#include <QJsonDocument>

#include "cnnimageclassifier.h"

class QClassifier : public QObject
{
    Q_OBJECT
public:
    explicit    QClassifier(QObject *parent = nullptr);
    void        loadResources(const cv::Ptr<cv::oirt::CNNImageClassifier> &_ptr);

signals:
    void        taskAccomplished(qintptr _taskid, const QByteArray &_info);

public slots:
    void        classify(qintptr _taskid, const QByteArray &_encimg);
    void        predict(qintptr _taskid, const QByteArray &_encimg);
    void        listLabels(qintptr _taskid);

private:
    cv::Ptr<cv::oirt::CNNImageClassifier> ptrrec;
    QJsonDocument::JsonFormat jsonformat;
};

#endif // QCLASSIFIER_H
