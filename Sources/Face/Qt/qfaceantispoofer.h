#ifndef QFACEANTISPOOFER_H
#define QFACEANTISPOOFER_H

#include <QObject>

#include "googlenetrecognizer.h"

class QFaceAntiSpoofer : public QObject
{
    Q_OBJECT

public:
    explicit QFaceAntiSpoofer(QObject *_parent=nullptr);
    bool loadResources(const QString &_labelsfilename, const QString &_prototxt, const QString &_caffemodel);
    enum faceType {Real, Spoofed};

signals:
    void predictionUpdated(int _label, double _distance, const QString &_labelInfo);

public slots:
    void predict(const cv::Mat &_imgmat);

private:
    cv::Ptr<cv::imgrec::CNNImageRecognizer> ptrrec;
};

#endif // QFACEANTISPOOFER_H
