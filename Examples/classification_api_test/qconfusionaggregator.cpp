#include "qconfusionaggregator.h"

#include <iostream>

QConfusionAggregator::QConfusionAggregator(const QStringList &_labels, QObject *parent) : QObject(parent)
{
    QMap<QString,unsigned long> _frequenciesmap;
    for(const auto _key: _labels)
        _frequenciesmap.insert(_key,0);
    for(const auto _key: _labels)
        confusiontable.insert(_key,_frequenciesmap);
}

void QConfusionAggregator::printResults()
{
    qInfo("Confusion table:");
    std::cout  << "P\\T";
    for(const auto &_key: confusiontable.keys())
        std::cout  << "\t" << _key.toUtf8().constData() ;
    std::cout << std::endl;
    for(const auto &_key: confusiontable.keys()) {
        std::cout << _key.toUtf8().constData();
        for(const auto &_subkey: confusiontable.value(_key).keys()) {
            std::cout << "\t" << confusiontable.value(_key).value(_subkey,0);
        }
        std::cout << std::endl;
    }
}

void QConfusionAggregator::update(const QString &_truelabel, const QString &_predicted)
{
    qInfo("  true: '%s', predicted: '%s'",_truelabel.toUtf8().constData(),_predicted.toUtf8().constData());
    confusiontable[_truelabel][_predicted] = confusiontable[_truelabel][_predicted] + 1;
}
