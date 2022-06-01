#ifndef QLABELHOLDER_H
#define QLABELHOLDER_H

#include <QObject>

class QLabelHolder : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(bool checked READ checked WRITE setChecked NOTIFY checkedChanged)

public:
    explicit QLabelHolder(const QString _name, QObject *parent = nullptr);

    QString name() const;
    void setName(const QString &_name);

    bool checked() const;
    void setChecked(const bool _checked);

public slots:
    void uncheck();

signals:
    void nameChanged(const QString &_name);
    void checkedChanged(const bool _checked);

private:
    QString m_name;
    bool    m_checked;
};

#endif // QLABELHOLDER_H
