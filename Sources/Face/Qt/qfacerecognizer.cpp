#include "qfacerecognizer.h"

QFaceRecognizer::QFaceRecognizer(QObject *parent) : QObject(parent)
{

}

void QFaceRecognizer::loadResources(const QString &_faceshapepredictormodel, const QString &_dlibfacedescriptor)
{
    ptrrec = cv::imgrec::createDlibFaceRecognizer(_faceshapepredictormodel.toUtf8().constData(),_dlibfacedescriptor.toUtf8().constData());
}

bool QFaceRecognizer::loadLabels(const QString &_labelsfilename)
{
    ptrrec->ImageRecognizer::load(_labelsfilename.toUtf8().constData());
    return !ptrrec->empty();
}

void QFaceRecognizer::setThreshold(double _val)
{
    ptrrec->setThreshold(_val);
}

void QFaceRecognizer::predict(cv::Mat _faceimg)
{
    int _label;
    double _distance;
    ptrrec->predict(_faceimg,_label,_distance);
    emit labelPredicted(_label,_distance,ptrrec->getLabelInfo(_label).c_str());
}
