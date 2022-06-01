#ifndef QCONFUSIONAGGREGATOR_H
#define QCONFUSIONAGGREGATOR_H

#include <QObject>
#include <QMap>

class QConfusionAggregator : public QObject
{
    Q_OBJECT
public:
    explicit QConfusionAggregator(const QStringList &_labels, QObject *parent = nullptr);


public slots:
    void update(const QString &_truelabel, const QString &_predicted);
    void printResults();

private:
    QMap<QString,QMap<QString,unsigned long>> confusiontable;
};

#endif // QCONFUSIONAGGREGATOR_H
