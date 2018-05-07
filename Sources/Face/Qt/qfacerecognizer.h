#ifndef QFACERECOGNIZER_H
#define QFACERECOGNIZER_H

#include <QObject>

#include "dlibfacerecognizer.h"

class QFaceRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit    QFaceRecognizer(QObject *parent = nullptr);
    void        loadResources(const QString &_faceshapepredictormodel, const QString &_dlibfacedescriptor);
    bool        loadLabels(const QString &_labelsfilename);

    QString     getLabelsfilename() const;
    void        setLabelsfilename(const QString &value);

signals:
    void        labelPredicted(int _label, double _distance, const QString &_labelInfo);
    void        taskAccomplished(qintptr _taskid, const QByteArray &_info);

public slots:
    void        setThreshold(double _val);
    void        predict(cv::Mat _faceimg);

    void        rememberLabel(qintptr _taskid, const QByteArray &_labelinfo, const QByteArray &_encimg);
    void        deleteLabel(qintptr _taskid, const QByteArray &_labelinfo);
    void        identifyImage(qintptr _taskid, const QByteArray &_encimg);

private:
    cv::Ptr<cv::imgrec::CNNImageRecognizer> ptrrec;
    QString labelsfilename;
};

#endif // QFACERECOGNIZER_H