#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QTcpSocket>
#include <windows.h>
#include <QDesktopWidget>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    text_for_clients_timer=new QTimer(this);
    textserver=new TextServer(this);
    connect(ui->textEdit,SIGNAL(keypressed(QKeyEvent *)),this,SLOT(keypressed(QKeyEvent *)));
    connect(textserver,SIGNAL(rx_from_client(QTcpSocket*)),this,SLOT(rx_from_client(QTcpSocket*)));
    connect(textserver,SIGNAL(info(QString)),this,SLOT(info(QString)));

    connect(text_for_clients_timer,SIGNAL(timeout()),this,SLOT(sendtext_to_clients()));
    on_checkBox_enable_clicked();

    ui->textEdit->setFocus();

    QTimer::singleShot(500,this,SLOT(keypaste()));

}

MainWindow::~MainWindow()
{
    delete ui;
}

//read info from other things
void  MainWindow::info(const QString &text)
{
    ui->textEdit_2->append(text);
}

//read any client data sent to us
void  MainWindow::rx_from_client(QTcpSocket *clientSocket)
{
    ui->textEdit_2->append(QString::fromUtf8(clientSocket->readAll()));
}

//send keypresses to all clients
void MainWindow::keypressed(QKeyEvent *e)
{
    text_for_clients+=e->text();
    text_for_clients_timer->start(100);
}

void MainWindow::sendtext_to_clients()
{
    text_for_clients_timer->stop();
    if(text_for_clients.isEmpty())return;
    textserver->tx_to_all_clients(text_for_clients);
    text_for_clients.clear();

    keypaste();

}

void MainWindow::keypaste()
{

    if(!ui->textEdit->hasFocus())
    {
        qDebug()<<"no focus wont send";
        return;
    }
    ui->textEdit->passthrough=true;
    keybd_event(VkKeyScan('t'),0x9e,0 , 0);
    keybd_event(VkKeyScan('t'),0x9e, KEYEVENTF_KEYUP,0);

}

void MainWindow::on_checkBox_enable_clicked()
{
    if(ui->checkBox_enable->isChecked())
    {
        QStringList strlist=ui->lineEdit_address->text().split(":");
        if(strlist.size()<2)
        {
            ui->checkBox_enable->setChecked(false);
            ui->textEdit_2->append("need address with \":\" ie 0.0.0.0:12345");
        }
         else
         {
            QHostAddress addr;
            addr.setAddress(strlist[0]);
            qDebug()<<addr.toString();
            textserver->bind(addr,strlist[1].toInt());
            ui->lineEdit_address->setEnabled(false);
            ui->textEdit->setFocus();
         }
    }
     else
     {
        textserver->close();
        ui->lineEdit_address->setEnabled(true);
     }
}
