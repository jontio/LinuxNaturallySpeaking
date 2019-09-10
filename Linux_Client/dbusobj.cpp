#include "dbusobj.h"
#include <QDebug>

DbusObj::DbusObj(QObject *parent) : QDBusAbstractAdaptor(parent)
{

}

QString DbusObj::test(QString test)
{
    qDebug()<<"test"<<test;
    return "hi "+test;
}

