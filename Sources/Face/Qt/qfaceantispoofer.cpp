#include "qfaceantispoofer.h"

QFaceAntiSpoofer::QFaceAntiSpoofer(QObject *_parent) : QObject(_parent)
{
}

bool QFaceAntiSpoofer::loadResources(const QString &_labelsfilename, const QString &_prototxt, const QString &_caffemodel)
{
    ptrrec = cv::imgrec::createGoogleNetRecognizer(_prototxt.toUtf8().constData(),_caffemodel.toUtf8().constData());
    ptrrec->ImageRecognizer::load(_labelsfilename.toUtf8().constData());
    return !ptrrec->empty();
}

void QFaceAntiSpoofer::predict(const cv::Mat &_imgmat)
{
    int _label;
    double _distance;
    ptrrec->predict(_imgmat,_label,_distance);
    emit predictionUpdated(_label,_distance,ptrrec->getLabelInfo(_label).c_str());
}
