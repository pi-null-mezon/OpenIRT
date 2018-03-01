#ifndef QFACERECOGNIZER_H
#define QFACERECOGNIZER_H

#include <QObject>

#include "dlibfacerecognizer.h"

class QFaceRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit QFaceRecognizer(QObject *parent = nullptr);
    bool loadResources(const QString &_labelsfilename, const QString &_faceshapepredictormodel, const QString &_dlibfacedescriptor);

signals:
    void labelPredicted(int _label, double _distance, const QString &_labelInfo);

public slots:
    void setThreshold(double _val);
    void predict(cv::Mat _faceimg);

private:
    cv::Ptr<cv::imgrec::CNNImageRecognizer> ptrrec;
};

#endif // QFACERECOGNIZER_H
