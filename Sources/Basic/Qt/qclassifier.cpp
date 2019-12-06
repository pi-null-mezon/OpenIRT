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

void QClassifier::predict(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(_encimg.isEmpty()) {
        _json["status"] = "Error";
        _json["info"]   = "Can not decode image";
    } else {
        cv::Mat _img = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
        if(_img.empty()) {
            _json["status"] = "Error";
            _json["info"]   = "Can not decode image";
        } else {
            float _conf;
            int _lbl, _error = 0;
            ptrrec->predict(_img,_lbl,_conf,&_error);
            if(_error == 0) {
                _json["status"] = "Success";
                _json["label"]  = ptrrec->getLabelInfo(_lbl).c_str();
                _json["conf"]   = _conf;
            } else {
                _json["status"] = "Error";
                _json["info"]   = ptrrec->getErrorInfo(_error).c_str();
            }
        }
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QClassifier::classify(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(_encimg.isEmpty()) {
        _json["status"] = "Error";
        _json["info"]   = "Can not decode image";
    } else {
        cv::Mat _img = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
        if(_img.empty()) {
            _json["status"] = "Error";
            _json["info"]   = "Can not decode image!";
        } else {
            std::vector<float> _vconf;
            int _error = 0;
            ptrrec->predict(_img,_vconf,&_error);
            if(_error == 0) {
                _json["status"] = "Success";
                for(size_t i = 0; i < _vconf.size(); ++i)
                    _json[ptrrec->getLabelInfo(static_cast<int>(i)).c_str()] = _vconf[i];
            } else {
                _json["status"] = "Error";
                _json["info"]   = ptrrec->getErrorInfo(_error).c_str();
            }
        }
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}


void QClassifier::listLabels(qintptr _taskid)
{
    QJsonObject _json;
    QJsonArray _jsonarray;
    for(const auto & _info: ptrrec->getLabelsInfo()) {
        _jsonarray.push_back(_info.second.c_str());
    }
    _json["status"] = "Success";
    _json["labels"] = _jsonarray;
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}
