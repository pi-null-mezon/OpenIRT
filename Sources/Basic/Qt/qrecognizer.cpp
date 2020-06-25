#include "qrecognizer.h"

#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgcodecs.hpp>

#include <QElapsedTimer>

QRecognizer::QRecognizer(QObject *parent) : QObject(parent),
    jsonformat(QJsonDocument::Indented),
    backuptimer(nullptr)
{    
}

void QRecognizer::loadResources(const cv::Ptr<cv::oirt::CNNImageRecognizer> &_ptr)
{
    ptrrec = _ptr;
}

bool QRecognizer::loadLabels(const QString &_labelsfilename)
{
    QElapsedTimer et;
    et.start();
    ptrrec->ImageRecognizer::load(_labelsfilename.toUtf8().constData());
    qInfo("Templates loading time: %u ms", static_cast<uint>(et.elapsed()));
    return !ptrrec->empty();   
}

void QRecognizer::setThreshold(double _val)
{
    ptrrec->setThreshold(_val);
}

void QRecognizer::rememberLabel(qintptr _taskid, const QByteArray &_labelinfo, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(_encimg.isEmpty()) {
        _json["status"]    = "Error";
        _json["message"]   = "Can not decode input image!";
    } else {
        // Conversion to base64 is necessary because user could pass markup symbols
        // like { or [ that will corrupt classifier's file storage
        QByteArray _encodedlabelinfo = _labelinfo.toBase64();

        int _label;
        auto _vlabels = ptrrec->getLabelsByString(_encodedlabelinfo.constData());
        if(_vlabels.size() > 0) {
           qDebug("This labelinfo already in use, so this example will be added to existed label");
           _label = _vlabels[0];
        } else {
           _label = ptrrec->nextfreeLabel();
        }
        std::vector<cv::Mat> _vmats(1,cv::Mat());
        _vmats[0] = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
        std::vector<int>     _vlbls(1,_label);

        int _error = 0;
        if(_vmats[0].empty() == false) {
            ptrrec->update(_vmats,_vlbls,false,&_error);
            if(_error == 0) {
                ptrrec->setLabelInfo(_label,_encodedlabelinfo.constData());
                backuptimer->start();
                _json["status"]    = "Success";
                _json["label"]     = _label;
                _json["labelinfo"] = _labelinfo.constData();
                _json["templates"] = ptrrec->labelTemplates(_label);
                _json["whitelist"] = ptrrec->isLabelWhitelisted(_label);
            } else {
                _json["status"]    = "Error";
                _json["code"]      = _error;
                _json["message"]   = ptrrec->getErrorInfo(_error).c_str();
            }
        } else {
            _json["status"]    = "Error";
            _json["message"]   = "Can not decode input image!";
        }
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::deleteLabel(qintptr _taskid, const QByteArray &_labelinfo)
{
    // Conversion to base64 is necessary because user could pass markup symbols
    // like { or [ that will corrupt classifier file storage
    QByteArray _encodedlabelinfo = _labelinfo.toBase64();

    QJsonObject _json;
    if(ptrrec->empty() == false) {
        auto _vlabels = ptrrec->getLabelsByString(_encodedlabelinfo.constData());
        if(ptrrec->remove(_vlabels) > 0) {
            backuptimer->start();
            _json["status"]  = "Success";
            _json["message"] = QString("%1 has been deleted").arg(_labelinfo.constData()).toUtf8().constData();
        } else {
            _json["status"]  = "Error";
            _json["message"] = QString("No %1 has been found in labels list!").arg(_labelinfo.constData()).toUtf8().constData();
        }
    } else {
        _json["status"] = "Error";
        _json["message"] = "Empty labels list, can not delete anything!";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::identifyImage(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(ptrrec->empty() == false) {
        if(ptrrec->emptyWhitelist() == false) {
            if(_encimg.isEmpty()) {
                _json["status"]    = "Error";
                _json["message"]   = "Can not decode input image!";
            } else {
                cv::Mat _faceimg = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
                int _label;
                double _distance;
                int _error = 0;
                ptrrec->ImageRecognizer::predict(_faceimg,_label,_distance,&_error);
                if(_error == 0) {
                    _json["status"]    = "Success";
                    _json["label"]     = _label;
                    // Conversion to base64 is necessary because user could pass markup symbols
                    // like { or [ that will corrupt classifier file storage
                    _json["labelinfo"] = QByteArray::fromBase64(ptrrec->getLabelInfo(_label).c_str()).constData();
                    _json["distance"]  = _distance;
                    _json["distancethresh"]    = ptrrec->getThreshold();
                } else {
                    _json["status"]    = "Error";
                    _json["code"]      = _error;
                    _json["message"]   = ptrrec->getErrorInfo(_error).c_str();
                }
            }
        } else {
            _json["status"]  = "Error";
            _json["message"] = "No labels in whitelist, can not identify anything!";
        }
    } else {
        _json["status"]  = "Error";
        _json["message"] = "Empty labels list, can not identify anything!";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::recognizeImage(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(ptrrec->empty() == false) {
        if(ptrrec->emptyWhitelist() == false) {
            if(_encimg.isEmpty()) {
                _json["status"]    = "Error";
                _json["message"]   = "Can not decode input image!";
            } else {
                cv::Mat _faceimg = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
                int _error = 0;
                std::vector<std::pair<int,double>> vpredictions = ptrrec->recognize(_faceimg,true,&_error);
                if(_error == 0) {
                    if(vpredictions.size() > 0) {
                        QJsonArray _jsonarray;
                        for(size_t i = 0; i < vpredictions.size(); ++i) {
                            if(vpredictions[i].second < ptrrec->getThreshold()) {
                                QJsonObject _tmpjson;
                                _tmpjson["label"]     = vpredictions[i].first;
                                // Conversion to base64 is necessary because user could pass markup symbols
                                // like { or [ that will corrupt classifier file storage
                                _tmpjson["labelinfo"] = QByteArray::fromBase64(ptrrec->getLabelInfo(vpredictions[i].first).c_str()).constData();
                                _tmpjson["distance"]  = vpredictions[i].second;
                                _tmpjson["distancethresh"] = ptrrec->getThreshold();
                                _jsonarray.push_back(_tmpjson);
                            }
                        }
                        if(_jsonarray.size() > 0) {
                            _json["status"] =    "Success";
                            _json["predictions"] = _jsonarray;
                        } else {
                            _json["status"]    = "Error";
                            _json["message"]   = "Can not find anything close enough to this image!";
                        }
                    }
                } else {
                    _json["status"]    = "Error";
                    _json["code"]      = _error;
                    _json["message"]   = ptrrec->getErrorInfo(_error).c_str();
                }
            }
        } else {
            _json["status"]  = "Error";
            _json["message"] = "No labels in whitelist, can not identify anything!";
        }
    } else {
        _json["status"]  = "Error";
        _json["message"] = "Empty labels list, can not identify anything!";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::verifyImage(qintptr _taskid, const QByteArray &_eimg, const QByteArray &_vimg)
{
    QJsonObject _json;
    if(_eimg.isEmpty() || _vimg.isEmpty()) {
        _json["status"]    = "Error";
        _json["message"]   = "Can not decode input image!";
    } else {
        cv::Mat _efaceimg = cv::imdecode(std::vector<unsigned char>(_eimg.begin(),_eimg.end()),cv::IMREAD_UNCHANGED);
        cv::Mat _vfaceimg = cv::imdecode(std::vector<unsigned char>(_vimg.begin(),_vimg.end()),cv::IMREAD_UNCHANGED);
        int _error = 0;
        double _distance = ptrrec->compare(_efaceimg,_vfaceimg, &_error);
        if(_error == 0) {
            _json["status"]         = "Success";
            _json["distance"]       = _distance;
            _json["distancethresh"] = ptrrec->getThreshold();
        } else {
            _json["status"]         = "Error";
            _json["message"]        = ptrrec->getErrorInfo(_error).c_str();
        }
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::updateWhitelist(qintptr _taskid, const QByteArray &_jsonwhitelist)
{
    QJsonObject _json;

    QJsonParseError _error;
    QJsonArray _jsonarray = QJsonDocument::fromJson(_jsonwhitelist,&_error).array();
    if(_error.error != QJsonParseError::NoError) {
        _json["status"]         = "Error";
        _json["message"]        = QString("JSON parsing error: %1").arg(_error.errorString());
    } else if(_jsonarray.size() == 0) {
        _json["status"]         = "Error";
        _json["message"]        = "Empty whitelist!";
    } else {
        std::vector<cv::String> _vlabelinfo(_jsonarray.size(),cv::String());
        for(int i = 0; i < _jsonarray.size(); ++i)
            _vlabelinfo[i] = _jsonarray[i].toString().toUtf8().toBase64().constData();

        ptrrec->setWhitelist(_vlabelinfo);
        backuptimer->start();
        _json["status"]      = "Success";
        _json["message"]     = "Whitelist has been updated";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::dropWhitelist(qintptr _taskid)
{
    QJsonObject _json;
    ptrrec->dropWhitelist();
    backuptimer->start();
    _json["status"]      = "Success";
    _json["message"]     = "Whitelist has been updated";
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QRecognizer::saveTemplatesOnDisk()
{
    if(ptrrec->empty() == false) {
        ptrrec->ImageRecognizer::save(getLabelsfilename().toUtf8().constData());
        backuptimer->stop();
    }
}

void QRecognizer::getLabelsList(qintptr _taskid)
{
    std::map<int,cv::String> _labelsInfo = ptrrec->getLabelsInfo();
    QJsonArray _jsonarray;
    for(auto const &_info : _labelsInfo) {
        QJsonObject _json;
        _json["label"]     = _info.first;
        // Conversion to base64 is necessary because user could pass markup symbols
        // like { or [ that will corrupt classifier file storage
        _json["labelinfo"] = QByteArray::fromBase64(_info.second.c_str()).constData();
        _json["templates"] = ptrrec->labelTemplates(_info.first);
        _json["whitelist"] = ptrrec->isLabelWhitelisted(_info.first);
        _jsonarray.push_back(_json);
    }
    QJsonObject _jsonobj{
                            {"status","Success"},
                            {"labels",_jsonarray}
                        };
    emit taskAccomplished(_taskid,QJsonDocument(_jsonobj).toJson(jsonformat));
}

QString QRecognizer::getLabelsfilename() const
{
    return labelsfilename;
}

void QRecognizer::setLabelsfilename(const QString &value)
{
    labelsfilename = value;
}

void QRecognizer::initBackupTimer()
{
    backuptimer = new QTimer(this);
    backuptimer->setInterval(5000);
    connect(backuptimer,SIGNAL(timeout()),this,SLOT(saveTemplatesOnDisk()));
}
