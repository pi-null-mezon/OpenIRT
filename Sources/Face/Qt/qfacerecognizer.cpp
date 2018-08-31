#include "qfacerecognizer.h"

#include <QJsonObject>
#include <QJsonArray>

#include <opencv2/imgcodecs.hpp>

QFaceRecognizer::QFaceRecognizer(QObject *parent) : QObject(parent),
    jsonformat(QJsonDocument::Indented)
{
}

void QFaceRecognizer::loadResources(const QString &_faceshapepredictormodel, const QString &_dlibfacedescriptor)
{
    ptrrec = cv::oirt::createDlibFaceRecognizer(_faceshapepredictormodel.toUtf8().constData(),_dlibfacedescriptor.toUtf8().constData());
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
    // Conversion to base64 is necessary because user could pass markup symbols
    // like { or [ that will corrupt classifier file storage
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
    QJsonObject _json;
    int _error = 0;
    if(_vmats[0].empty() == false) {
        ptrrec->update(_vmats,_vlbls,false,&_error);
        if(_error == 0) {           
            ptrrec->setLabelInfo(_label,_encodedlabelinfo.constData());
            ptrrec->ImageRecognizer::save(getLabelsfilename().toUtf8().constData());
            _json["status"]    = "Success";
            _json["label"]     = _label;
            _json["labelinfo"] = _labelinfo.constData();
            _json["templates"] = ptrrec->labelTemplates(_label);
        } else {
            _json["status"]    = "Error";
            _json["message"]   = ptrrec->getErrorInfo(_error).c_str();
        }
    } else {
        _json["status"]    = "Error";
        _json["message"]   = "Can not decode input image!";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QFaceRecognizer::deleteLabel(qintptr _taskid, const QByteArray &_labelinfo)
{
    // Conversion to base64 is necessary because user could pass markup symbols
    // like { or [ that will corrupt classifier file storage
    QByteArray _encodedlabelinfo = _labelinfo.toBase64();

    QJsonObject _json;
    if(ptrrec->empty() == false) {
        auto _vlabels = ptrrec->getLabelsByString(_encodedlabelinfo.constData());
        if(ptrrec->remove(_vlabels) > 0) {
            ptrrec->ImageRecognizer::save(getLabelsfilename().toUtf8().constData());
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

void QFaceRecognizer::identifyImage(qintptr _taskid, const QByteArray &_encimg)
{
    QJsonObject _json;
    if(ptrrec->empty() == false) {
        cv::Mat _faceimg = cv::imdecode(std::vector<unsigned char>(_encimg.begin(),_encimg.end()),cv::IMREAD_UNCHANGED);
        int _label;
        double _distance;
        int _error = 0;
        ptrrec->predict(_faceimg,_label,_distance,&_error);
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
            _json["message"]   = ptrrec->getErrorInfo(_error).c_str();
        }
    } else {
        _json["status"]  = "Error";
        _json["message"] = "Empty labels list, can not identify anything!";
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QFaceRecognizer::verifyImage(qintptr _taskid, const QByteArray &_eimg, const QByteArray &_vimg)
{
    cv::Mat _efaceimg = cv::imdecode(std::vector<unsigned char>(_eimg.begin(),_eimg.end()),cv::IMREAD_UNCHANGED);
    cv::Mat _vfaceimg = cv::imdecode(std::vector<unsigned char>(_vimg.begin(),_vimg.end()),cv::IMREAD_UNCHANGED);
    int _error = 0;
    double _distance = ptrrec->compare(_efaceimg,_vfaceimg, &_error);
    QJsonObject _json;
    if(_error == 0) {
        _json["status"]         = "Success";
        _json["distance"]       = _distance;
        _json["distancethresh"] = ptrrec->getThreshold();
    } else {
        _json["status"]         = "Error";
        _json["message"]        = ptrrec->getErrorInfo(_error).c_str();
    }
    emit taskAccomplished(_taskid,QJsonDocument(_json).toJson(jsonformat));
}

void QFaceRecognizer::getLabelsList(qintptr _taskid)
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
        _jsonarray.push_back(_json);
    }
    QJsonObject _jsonobj{
                            {"status","Success"},
                            {"labels",_jsonarray}
                        };
    emit taskAccomplished(_taskid,QJsonDocument(_jsonobj).toJson(jsonformat));
}

QString QFaceRecognizer::getLabelsfilename() const
{
    return labelsfilename;
}

void QFaceRecognizer::setLabelsfilename(const QString &value)
{
    labelsfilename = value;
}
