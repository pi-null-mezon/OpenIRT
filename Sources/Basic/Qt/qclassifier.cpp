#include "qclassifier.h"

#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgcodecs.hpp>

#include <QElapsedTimer>

QClassifier::QClassifier(QObject *parent) : QObject(parent),
    jsonformat(QJsonDocument::Indented)
{    
}

void QClassifier::loadResources(const cv::Ptr<cv::oirt::CNNImageClassifier> &_ptr)
{
    ptrrec = _ptr;
}

void QClassifier::classifyImage(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    cv::Mat _img = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
    std::vector<float> _vconf;
    int _error = 0;
    ptrrec->predict(_img,_vconf,&_error);
    if(_error == 0) {
        _json["status"] = "Success";
        for(size_t i = 0; i < _vconf.size(); ++i)
            _json[ptrrec->getLabelInfo(i).c_str()] = _vconf[i];
    } else {
        _json["status"] = "Error";
        _json["info"]   = ptrrec->getErrorInfo(_error).c_str();
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}
