#ifndef TEXTSERVER_H
#define TEXTSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QSslKey>
#include <QSslCertificate>

class TextServer : public QTcpServer
{
    Q_OBJECT

public:
    TextServer(QObject *parent = 0);
    bool bind(const QHostAddress &address = QHostAddress::Any, quint16 port = 0);

public slots:
    void tx_to_all_clients(QString text);

signals:
    void rx_from_client(QTcpSocket *clientSocket);
    void info(const QString &text);

private:
    QSslKey key;
    QSslCertificate cert;
    QList<QTcpSocket*> peerSockets;

private slots:
    void sslErrors(const QList<QSslError> &errors);
    void link();
    void rx();
    void disconnected();

protected:
    void incomingConnection(qintptr socketDescriptor);
};


#endif // TEXTSERVER_H
