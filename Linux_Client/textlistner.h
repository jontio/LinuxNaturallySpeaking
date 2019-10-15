#ifndef TEXTLISTNER_H
#define TEXTLISTNER_H

#include <QObject>
#include <QSslSocket>
#include <QPointer>

class TextListner : public QObject
{
    Q_OBJECT

public:
    explicit TextListner(QObject *parent = nullptr);
    ~TextListner();
public slots:
    void connectToServer(QString hostname,qint16 port);
    void setCredentials(QString CaRemote,QString CaLocal,QString KeyPrivate);
    void disconnectFromServer();
signals:
    void disconnected(void);
    void text(QString text);
    void text_voice(QString text);
    void connectToServerResult(bool connectOk);
    void setCredentialsResult(bool credentialsSetOk);
private slots:
    void sslErrors(const QList<QSslError> &errors);
    void rx(void);
    void serverDisconnect(void);
    void ready_con();
    void ready_enc();
private:
    QPointer<QSslSocket> server;
};


#endif // TEXTLISTNER_H
