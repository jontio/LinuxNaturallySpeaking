#ifndef DBUSOBJ_H
#define DBUSOBJ_H

#include <QObject>
#include <QDBusAbstractAdaptor>

class DbusObj : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.zapto.jontio.thing_for_screen_text")
public:
    explicit DbusObj(QObject *parent = nullptr);

signals:

public slots:
    QString test(QString test);
};

#endif // DBUSOBJ_H
