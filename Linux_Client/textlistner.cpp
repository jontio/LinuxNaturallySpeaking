#include "textlistner.h"
#include <QSslKey>
#include <QSslCertificate>
#include <QFile>
#include <QThread>

TextListner::TextListner(QObject *parent) : QObject(parent)
{
}

TextListner::~TextListner()
{
    if(!server.isNull())server->deleteLater();
}

void TextListner::setCredentials(QString CaRemote,QString CaLocal,QString KeyPrivate)
{

    if((!QFile::exists(CaRemote))
            ||(!QFile::exists(CaLocal))
            ||(!QFile::exists(KeyPrivate)))
    {
        emit text("can't find credential files");
        emit setCredentialsResult(false);
        return;
    }

    if(!server.isNull())server->deleteLater();
    server=new QSslSocket(this);

    connect(server, SIGNAL(readyRead()), this, SLOT(rx()));
    connect(server, SIGNAL(disconnected()), this, SLOT(serverDisconnect()));
    connect(server, SIGNAL(sslErrors(QList<QSslError>)), this, SLOT(sslErrors(QList<QSslError>)));

    //him
    //this says who we can trust
    server->addCaCertificates(CaRemote);//him

    //us
    //this is who we are
    server->setPrivateKey(KeyPrivate);
    server->setLocalCertificate(CaLocal);

    //this says dont worry about host name
    server->setPeerVerifyName("127.0.0.1");

    server->setPeerVerifyMode(QSslSocket::VerifyPeer);

    connect(server, SIGNAL(encrypted()), this, SLOT(ready_enc()));
    connect(server, SIGNAL(connected()), this, SLOT(ready_con()));

    emit setCredentialsResult(true);
}

void TextListner::ready_enc()
{
    emit text("encrypted connected");
}

void TextListner::ready_con()
{
    emit text("unencrypted conected, negotiating encryption");
}

void TextListner::connectToServer(QString hostname, qint16 port)
{
    server->connectToHostEncrypted(hostname, port);  // FQDN in red_local.pem is set to 127.0.0.1.  If you change this, it will not authenticate.
    emit text("connecting");
    if (server->waitForEncrypted(5000))
    {
        emit text(server->peerName());
        QSslCertificate peercert=server->peerCertificate();
        emit text(peercert.toText());
        server->write("Authentication Suceeded");
        emit connectToServerResult(true);
    }
    else
    {
        emit text("Unable to connect to server");
        server->disconnectFromHost();
        emit connectToServerResult(false);
    }
}

void TextListner::disconnectFromServer()
{
    server->disconnectFromHost();
}

void TextListner::sslErrors(const QList<QSslError> &errors)
{
    foreach (const QSslError &error, errors)
    {
        qDebug() << error.errorString();
        emit text(error.errorString());
    }
}

void TextListner::serverDisconnect(void)
{
    emit text("Server disconnected");
    disconnected();
}

void TextListner::rx(void)
{
    emit text_voice(server->readAll());
    //qDebug() << server->readAll();
}


