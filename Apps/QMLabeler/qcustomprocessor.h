#ifndef QCUSTOMPROCESSOR_H
#define QCUSTOMPROCESSOR_H

#include <QObject>
#include <QSettings>
#include <QQmlApplicationEngine>
#include <QCoreApplication>
#include <QTranslator>

#include "qlabelholder.h"
#include "qsimplemaintenancetool.h"

class QCustomProcessor : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString language READ language WRITE setLanguage NOTIFY languageChanged)
    Q_PROPERTY(QString inputdir READ inputdir WRITE setinputdir NOTIFY inputdirChanged)
    Q_PROPERTY(QString outputdir READ outputdir WRITE setoutputdir NOTIFY outputdirChanged)
    Q_PROPERTY(float updtdownloadprogress READ updtdownloadprogress WRITE setUpdtdownloadprogress NOTIFY updtdownloadprogressChanged)
    Q_PROPERTY(QString updtsrvaddr READ updtsrvaddr WRITE setUpdtsrvaddr NOTIFY updtsrvaddrChanged)
    Q_PROPERTY(int updtsrvport READ updtsrvport WRITE setUpdtsrvport NOTIFY updtsrvportChanged)

public:
    explicit QCustomProcessor(QQmlApplicationEngine *_qmlengine, QObject *parent = nullptr);

    QString inputdir() const;
    void setinputdir(const QString &_inputdir);

    QString outputdir() const;
    void setoutputdir(const QString &_outputdir);

    void retranslate(const QString &_language);
    void updateAppInfo();

    QString language() const;
    void setLanguage(const QString &_language);

    int currentfileslistpos() const;
    void setCurrentfileslistpos(const int _currentfileslistpos);

    float updtdownloadprogress() const;
    void setUpdtdownloadprogress(const float _updtdownloadprogress);

    void checkForUpdates();

    QString updtsrvaddr() const;
    void setUpdtsrvaddr(const QString &updtsrvaddr);

    int updtsrvport() const;
    void setUpdtsrvport(const int _port);

signals:
    void updtsrvaddrChanged(const QString &_addr);
    void updtsrvportChanged(const int _port);
    void updtdownloadprogressChanged(float _value);
    void languageChanged(const QString &_language);
    void inputdirChanged(const QString &_inputdir);
    void outputdirChanged(const QString &_outputdir);
    void filenameUpdated(const QVariant &_filename);
    void infoUpdated(const QVariant &_info);
    void error(const QVariant &_error);

private:
    void setupMaintenanceTool();
    void openNextImage();
    void createLabelsList();
    void readFilesList();

    QSettings *settings;
    QQmlApplicationEngine *qmlengine;
    QTranslator translator;
    QList<QObject*> listofclasslabels;
    QStringList filesnameslist;
    QString filename;
    QSimpleMaintenanceTool qsmt;
    float m_updtdownloadprogress;
};

#endif // QCUSTOMPROCESSOR_H
