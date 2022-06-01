#include "qlabelholder.h"

QLabelHolder::QLabelHolder(const QString _name, QObject *parent) : QObject(parent),
    m_name(_name),
    m_checked(false)
{

}

QString QLabelHolder::name() const
{
    return m_name;
}

void QLabelHolder::setName(const QString &_name)
{
    if(_name != name()) {
        m_name = _name;
        emit nameChanged(_name);
    }
}

bool QLabelHolder::checked() const
{
    return m_checked;
}

void QLabelHolder::setChecked(const bool _checked)
{
    if(_checked != checked()) {
        //qDebug("%s: %s",name().toUtf8().constData(),_checked ? "checked" : "unchecked");
        m_checked = _checked;
        emit checkedChanged(_checked);
    }
}

void QLabelHolder::uncheck()
{
    setChecked(false);
}

