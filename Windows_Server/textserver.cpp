#include "textserver.h"
#include <QSslSocket>
#include <QFile>

TextServer::TextServer(QObject *parent) : QTcpServer(parent)
{
    connect(this, SIGNAL(newConnection()), this, SLOT(link()));
}

bool TextServer::bind(const QHostAddress &address, quint16 port)
{

    if(isListening())
    {
        close();
    }

    //load us
    //this is who we are
    QFile keyFile("certificates/red_local.key");
    keyFile.open(QIODevice::ReadOnly);
    key = QSslKey(keyFile.readAll(), QSsl::Rsa);
    keyFile.close();
    QFile certFile("certificates/red_local.pem");
    certFile.open(QIODevice::ReadOnly);
    cert = QSslCertificate(certFile.readAll());
    certFile.close();

    if (!listen(address, port))
    {
        emit info("Unable to bind to network port");
        return false;
    }

    emit info("Listening to "+address.toString()+":"+QString::number(port));
    return true;
}

void TextServer::incomingConnection(qintptr socketDescriptor)
{

    emit info("incomingConnection");


    QSslSocket *sslSocket = new QSslSocket(this);

    connect(sslSocket, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));
    sslSocket->setSocketDescriptor(socketDescriptor);

    //him
    //this says who we can trust
    sslSocket->addCaCertificates("certificates/blue_ca.pem");

    //us
    //this is who we are
    sslSocket->setPrivateKey(key);
    sslSocket->setLocalCertificate(cert);



//this says dont worry about host name
sslSocket->setPeerVerifyName("127.0.0.1");



    sslSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);

    sslSocket->startServerEncryption();

    addPendingConnection(sslSocket);
}

void TextServer::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
    {
        emit info(error.errorString());
    }
}

void TextServer::link()
{
    emit info("newConnection");
    QTcpSocket *clientSocket;
    clientSocket = nextPendingConnection();
    connect(clientSocket, SIGNAL(readyRead()), this, SLOT(rx()));
    connect(clientSocket, SIGNAL(disconnected()), this, SLOT(disconnected()));
    peerSockets.push_back(clientSocket);
}

void TextServer::rx()
{
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    emit rx_from_client(clientSocket);


  //  qDebug() << clientSocket->readAll();
   // clientSocket->write("Server says Hello");
}

void TextServer::disconnected()
{
    emit info("Client Disconnected");
    QTcpSocket* clientSocket = qobject_cast<QTcpSocket*>(sender());
    peerSockets.removeAll(clientSocket);
    clientSocket->deleteLater();
}

void TextServer::tx_to_all_clients(QString text)
{
    foreach(QTcpSocket* clientSocket, peerSockets)
    {
        clientSocket->write(text.toUtf8());
    }
}
