#ifndef QFACERECOGNIZER_H
#define QFACERECOGNIZER_H

#include <QObject>

#include "dlibfacerecognizer.h"

class QFaceRecognizer : public QObject
{
    Q_OBJECT
public:
    explicit QFaceRecognizer(QObject *parent = nullptr);
    void loadResources(const QString &_faceshapepredictormodel, const QString &_dlibfacedescriptor);
    bool loadLabels(const QString &_labelsfilename);

signals:
    void labelPredicted(int _label, double _distance, const QString &_labelInfo);

public slots:
    void setThreshold(double _val);
    void predict(cv::Mat _faceimg);

private:
    cv::Ptr<cv::imgrec::CNNImageRecognizer> ptrrec;
};

#endif // QFACERECOGNIZER_H
