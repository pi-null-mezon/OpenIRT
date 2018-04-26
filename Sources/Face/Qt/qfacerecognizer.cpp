#include "qfacerecognizer.h"

#include <opencv2/imgcodecs.hpp>

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

void QFaceRecognizer::rememberLabel(qintptr _taskid, const QByteArray &_labelinfo, const QByteArray &_encimg)
{
    int _label;
    auto _vlabels = ptrrec->getLabelsByString(_labelinfo.constData());
    if(_vlabels.size() > 0) {
       qDebug("This labelinfo already in use, so this example will be added to existed label");
       _label =_vlabels[0];
    } else {
       _label = ptrrec->nextfreeLabel();
    }

    std::vector<cv::Mat> _vmats(1,cv::Mat());
    _vmats[0] = std::move(cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED));
    std::vector<int>     _vlbls(1,_label);
    if(_vmats[0].empty() == false) {
       ptrrec->update(_vmats,_vlbls,false);
       ptrrec->setLabelInfo(_label,_labelinfo.constData());
       ptrrec->ImageRecognizer::save(getLabelsfilename().toUtf8().constData());
       emit taskAccomplished(_taskid,QString("{"
                                             "status: \"Success\","
                                             "label: %1,"
                                             "labelinfo: \"%2\""
                                             "}").arg(QString::number(_label),_labelinfo.constData()).toUtf8());
    } else {
       emit taskAccomplished(_taskid,QString("{"
                                             "status: \"Error\","
                                             "message: \"Can not decode input image!\""
                                             "}").toUtf8());
    }
}

void QFaceRecognizer::deleteLabel(qintptr _taskid, const QByteArray &_labelinfo)
{
    if(ptrrec->empty() == false) {
        auto _vlabels = ptrrec->getLabelsByString(_labelinfo.constData());
        ptrrec->remove(_vlabels);
        ptrrec->ImageRecognizer::save(getLabelsfilename().toUtf8().constData());
        emit taskAccomplished(_taskid,QString("{"
                                              "status: \"Success\","
                                              "message: \"%1\" has been deleted"
                                              "}").arg(_labelinfo.constData()).toUtf8());
    } else {
        emit taskAccomplished(_taskid,QString("{"
                                              "status: \"Error\","
                                              "message: \"Empty labels list, can not delete anything!\""
                                              "}").toUtf8());
    }
}

void QFaceRecognizer::identifyImage(qintptr _taskid, const QByteArray &_encimg)
{
    if(ptrrec->empty() == false) {
        cv::Mat _faceimg = std::move(cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED));
        int _label;
        double _distance;
        ptrrec->predict(_faceimg,_label,_distance);
        emit taskAccomplished(_taskid,QString("{"
                                              "status: \"Success\","
                                              "label: %1,"
                                              "labelinfo: \"%2\","
                                              "distance: %3"
                                              "}").arg(QString::number(_label),QString(ptrrec->getLabelInfo(_label).c_str()),QString::number(_distance,'f',3)).toUtf8());
    } else {
        emit taskAccomplished(_taskid,QString("{"
                                              "status: \"Error\","
                                              "message: \"Empty labels list, can not identify anything!\""
                                              "}").toUtf8());
    }
}

QString QFaceRecognizer::getLabelsfilename() const
{
    return labelsfilename;
}

void QFaceRecognizer::setLabelsfilename(const QString &value)
{
    labelsfilename = value;
}
