#ifndef TEXTLISTNER_H
#define TEXTLISTNER_H

#include <QObject>
#include <QSslSocket>

class TextListner : public QObject
{
    Q_OBJECT

public:
    explicit TextListner(QObject *parent = nullptr);
    bool connectToServer(QString hostname,qint16 port);
    void disconnectFromServer();

private:
    QSslSocket *server;

signals:
    void disconnected(void);
    void text(QString text);
    void text_voice(QString text);

private slots:
    void sslErrors(const QList<QSslError> &errors);
    void rx(void);
    void serverDisconnect(void);
    void ready_con();
    void ready_enc();
};


#endif // TEXTLISTNER_H
