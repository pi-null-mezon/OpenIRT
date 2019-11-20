#include "qcustomprocessor.h"

#include <QQmlContext>
#include <QStandardPaths>
#include <QLocale>
#include <QDir>
#include <QTimer>
#include <QVariant>
#include <QDesktopServices>

QCustomProcessor::QCustomProcessor(QQmlApplicationEngine *_qmlengine, QObject *parent) : QObject(parent),
    qmlengine(_qmlengine),
    m_updtdownloadprogress(0)
{
    QDir _dir(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation).append("/%1").arg(APP_NAME));
    if(!_dir.exists())
        _dir.mkpath(_dir.absolutePath());
    settings = new QSettings(_dir.absolutePath().append("/%1.ini").arg(APP_NAME),QSettings::IniFormat,this);
    retranslate(language());
    setupMaintenanceTool();
    checkForUpdates();
    qmlengine->rootContext()->setContextProperty("customsettings",this);    
    connect(this,SIGNAL(filenameUpdated(QVariant)),qmlengine->rootObjects().at(0),SLOT(openImage(QVariant)));
    connect(this,SIGNAL(infoUpdated(QVariant)),qmlengine->rootObjects().at(0),SLOT(showInfo(QVariant)));
    connect(this,SIGNAL(allFilesLabeled()),qmlengine->rootObjects().at(0),SLOT(dropImage()));
    connect(this,SIGNAL(askUpdateDialog(QVariant,QVariant,QVariant)),qmlengine->rootObjects().at(0),SLOT(showUpdateDialog(QVariant,QVariant,QVariant)));
    connect(qmlengine->rootObjects().at(0)->findChild<QObject*>("UpdateDialog"),SIGNAL(accepted()),this,SLOT(update()));
    connect(this,SIGNAL(error(QVariant)),qmlengine->rootObjects().at(0),SLOT(showError(QVariant)));
    createLabelsList();
    openNextImage();
}

void QCustomProcessor::updateAppInfo()
{
    if(qmlengine) {
       QMetaObject::invokeMethod(qmlengine->rootObjects().at(0),"setAppTitle",Q_ARG(QVariant,QString("%1 v.%2").arg(APP_NAME,APP_VERSION)));
       QMetaObject::invokeMethod(qmlengine->rootObjects().at(0),"setAppAbout",Q_ARG(QVariant,APP_NAME),
                                                                              Q_ARG(QVariant,APP_VERSION),
                                                                              Q_ARG(QVariant,tr("Приложение предназначено для ручной разметки изображений")),
                                                                              Q_ARG(QVariant,tr("Свободное программное обеспечение с отктрытым исходным кодом. Модифицируйте и используйте по своему усмотрению. Никаких гарантий не предоставляется")),
                                                                              Q_ARG(QVariant,tr("<a href='mailto:taransanya@mail.ru?subject=Вопрос по программе %1'>Обратная связь</a>").arg(APP_NAME)));
   }
}

void QCustomProcessor::retranslate(const QString &_language)
{
    if(_language == "Русский")
        QLocale::setDefault(QLocale(QLocale::Russian));
    else if(_language == "English")
        QLocale::setDefault(QLocale(QLocale::English,QLocale::UnitedKingdom));

    QCoreApplication::removeTranslator(&translator);
    bool _is_loaded = translator.load(QString(":/%1.qm").arg(_language));
    Q_UNUSED(_is_loaded)
    qDebug("%s %s",_language.toUtf8().constData(), _is_loaded ? "is loaded" : "is not loaded");
    QCoreApplication::installTranslator(&translator);
    updateAppInfo();
    if(qmlengine)
       qmlengine->retranslate();
}

QString QCustomProcessor::language() const
{
    return settings->value("Language","Русский").toString();
}

void QCustomProcessor::setLanguage(const QString &_language)
{
    if(language() != _language) {
        settings->setValue("Language",_language);
        retranslate(_language);
        emit languageChanged(_language);
    }
}

QString QCustomProcessor::inputdir() const
{
    return settings->value("InputDir").toString();
}

void QCustomProcessor::setinputdir(const QString &_inputdir)
{
    if(_inputdir != inputdir()) {
        settings->setValue("InputDir",_inputdir);
        setCurrentfileslistpos(0);
        filesnameslist.clear();
        openNextImage();
        emit inputdirChanged(_inputdir);
    }
}

QString QCustomProcessor::outputdir() const
{
    return settings->value("OutputDir").toString();
}

void QCustomProcessor::setoutputdir(const QString &_outputdir)
{
    if(_outputdir != outputdir()) {
        settings->setValue("OutputDir",_outputdir);
        createLabelsList();
        emit outputdirChanged(_outputdir);
    }
}

void QCustomProcessor::createLabelsList() {
    if(qmlengine && !outputdir().isEmpty()) {
        for(auto qobj: listofclasslabels)
            qobj->deleteLater();
        listofclasslabels.clear();
        QDir _dir(outputdir().section("file:///",-1,-1));
        QStringList _subdirsnames = _dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for(const auto &_subdirname: _subdirsnames) {
            QLabelHolder *_lblholder = new QLabelHolder(_subdirname,this);
            connect(_lblholder,&QLabelHolder::checkedChanged,[_lblholder,this](bool _checked) {
                if(_checked && (currentfileslistpos() < filesnameslist.size())) {
                    QFile::copy(inputdir().section("file:///",-1,-1).append("/%1").arg(filename),
                                outputdir().section("file:///",-1,-1).append("/%1/%2").arg(_lblholder->name(),filename));
                    setCurrentfileslistpos(currentfileslistpos()+1);
                    openNextImage();
                    QTimer::singleShot(300,_lblholder,SLOT(uncheck()));
                }
            });
            listofclasslabels.push_back(_lblholder);
        }
        qmlengine->rootContext()->setContextProperty("labels",QVariant::fromValue(listofclasslabels));
        if(listofclasslabels.size() == 0)
            emit error(tr("В директории для размеченных изображений не найдено ни одной субдиректории!"));
        else
            emit error(QString()); // to drop error message
    }
}

void QCustomProcessor::readFilesList()
{
    QDir _dir(QString(inputdir()).section("file:///",-1,-1));
    QStringList _extensions;
    _extensions << "*.jpg" << "*.jpeg" << "*.png" << "*.gif" << "*.bmp";
    filesnameslist = _dir.entryList(_extensions,QDir::Files|QDir::NoDotAndDotDot);
}

int QCustomProcessor::currentfileslistpos() const
{
    return settings->value("FilesPosition").toInt();
}

void QCustomProcessor::setCurrentfileslistpos(const int _currentfileslistpos)
{
    if(_currentfileslistpos != currentfileslistpos()) {
        settings->setValue("FilesPosition",_currentfileslistpos);
    }
}

void QCustomProcessor::openNextImage() {
    if(filesnameslist.size() == 0)
        readFilesList();

    if(currentfileslistpos() < filesnameslist.size()) {
        emit infoUpdated(tr("Файлов в директории с исходниками: %1, текущая позиция: %2").arg(QString::number(filesnameslist.size()),QString::number(currentfileslistpos())));
        filename = filesnameslist.at(currentfileslistpos());
        emit filenameUpdated(inputdir().append("/%1").arg(filename));
    } else {
        emit infoUpdated(tr("Все файлы были размечены, выберите другую директорию (свайп слеава направо)"));
        emit allFilesLabeled();
    }
}

void QCustomProcessor::setupMaintenanceTool()
{
    connect(&qsmt,&QSimpleMaintenanceTool::checked,[this](const QList<smt::Version> &_versions){
        // If this slot is called then _versions list is not empty
        const smt::Version &_lastversion = _versions.at(0); // greatest available
        if(_lastversion.version > APP_VERSION) {            
            updateversion = _lastversion.version;
            updatechangelog = _lastversion.changelog;
            qsmt.download(_lastversion.url);
            connect(&qsmt,&QSimpleMaintenanceTool::downloadProgress,[this](const QString &_url, qint64 bytesReceived, qint64 bytesTotal){
                Q_UNUSED(_url)
                setUpdtdownloadprogress(static_cast<float>(bytesReceived)/bytesTotal);
            });
        }
    });
    connect(&qsmt,&QSimpleMaintenanceTool::downloaded,[this](const QString &_filename){
        qDebug("Downloaded to '%s'",_filename.toUtf8().constData());
        updatefilename = _filename;
        emit askUpdateDialog(APP_NAME,updateversion,updatechangelog);
    });
    connect(&qsmt,&QSimpleMaintenanceTool::error,[this](const QString &_error){
        qDebug("%s",_error.toUtf8().constData());
    });
}

float QCustomProcessor::updtdownloadprogress() const
{
    return m_updtdownloadprogress;
}

void QCustomProcessor::setUpdtdownloadprogress(const float _updtdownloadprogress)
{
    if(updtdownloadprogress() != _updtdownloadprogress) {
        m_updtdownloadprogress = _updtdownloadprogress;
        emit updtdownloadprogressChanged(_updtdownloadprogress);
    }
}

void QCustomProcessor::checkForUpdates()
{
    qsmt.check(QString("http://%1:%2/Releases/updates.json").arg(updtsrvaddr(),QString::number(updtsrvport())));
}

int QCustomProcessor::updtsrvport() const
{
    return settings->value("UpdateServer/Port",80).toInt();
}

void QCustomProcessor::setUpdtsrvport(const int _port)
{
    if(updtsrvport() != _port) {
        settings->setValue("UpdateServer/Port",_port);
        emit updtsrvportChanged(_port);
    }
}

void QCustomProcessor::update()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(updatefilename));
    qApp->quit();
}

QString QCustomProcessor::updtsrvaddr() const
{
    return settings->value("UpdateServer/Address","10.0.192.47").toString();
}

void QCustomProcessor::setUpdtsrvaddr(const QString &_updtsrvaddr)
{
    if(updtsrvaddr() != _updtsrvaddr) {
        settings->setValue("UpdateServer/Address",_updtsrvaddr);
        emit updtsrvaddrChanged(_updtsrvaddr);
    }
}

