#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtDBus>
#include <QDBusConnection>
#include <QHostInfo>
#include <QHostAddress>
#include <QMenu>

//seems this cant be placed in mainwindow.h else we get errors
extern "C"
{
#include <xdo.h>
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    audio_listening=true;
    ignore_close=true;

    connect(ui->action_Quit,SIGNAL(triggered()),this,SLOT(CloseAccept()));

    ui->pushButton->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);

    ui->textEditmain->append("started");
    textlistner=new TextListner();
    textlistner_thread = new QThread(this);
    textlistner->moveToThread(textlistner_thread);
    textlistner_thread->start();

    connect(this,SIGNAL(connectToServer(QString,qint16)),textlistner,SLOT(connectToServer(QString,qint16)));
    connect(this,SIGNAL(setCredentials(QString,QString,QString)),textlistner,SLOT(setCredentials(QString,QString,QString)));
    connect(this,SIGNAL(disconnectFromServer()),textlistner,SLOT(disconnectFromServer()));

    connect(textlistner,SIGNAL(text(QString)),this,SLOT(text(QString)));
    connect(textlistner,SIGNAL(text_voice(QString)),this,SLOT(text_voice(QString)));
    connect(textlistner,SIGNAL(disconnected()),this,SLOT(resetbuttonsstates()));
    connect(textlistner,SIGNAL(connectToServerResult(bool)),this,SLOT(connectToServerResult(bool)));
    connect(textlistner,SIGNAL(setCredentialsResult(bool)),this,SLOT(setCredentialsResult(bool)));

    qDebug()<<"my window id"<<QCoreApplication::applicationPid();

    QSettings settings("jontisoft","lns");

    ui->lineEdit_address->setText(settings.value("lineEdit_address","192.168.65.129:13633").toString());
    ui->lineEdit_credentials_folder->setText(settings.value("lineEdit_credentials_folder","cerdentials").toString());
    ui->checkBox_keepconnected->setChecked(settings.value("checkBox_keepconnected",false).toBool());

    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(mousetimeout()));

}

void MainWindow::onTrayActivate(QSystemTrayIcon::ActivationReason reason)
{
    qDebug()<<"onTrayActivate";
    switch (reason)
    {
    case QSystemTrayIcon::Context:
    {
        QMenu menu(this);
        menu.addAction("&Show", this, SLOT(onTrayShow()));
        menu.addSeparator();
        menu.addAction("E&xit", this, SLOT(onTrayExit()));

        menu.exec(QCursor::pos());
    }
        break;

    case QSystemTrayIcon::DoubleClick:
    {
        if(isVisible())toTray();
         else onTrayShow();
    }
        break;
    case QSystemTrayIcon::Trigger:
    {
        if(isVisible())toTray();
         else onTrayShow();
    }
        break;

    default: break;
    }
}

void MainWindow::onTrayExit()
{
    qDebug()<<"exit";
    if(trayIcon)trayIcon->deleteLater();
    ignore_close=false;
    close();
}

void MainWindow::onTrayShow()
{
    qDebug()<<"show";
   // if(trayIcon)trayIcon->deleteLater();
    show();
}

void MainWindow::CloseAccept()
{
    ignore_close=false;
    close();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    qDebug()<<"closeEvent";
    if(ignore_close)
    {
        event->ignore();
        toTray();
    }
     else
     {
        QSettings settings("jontisoft","lns");
        if(ui->lineEdit_address->text()!=(settings.value("lineEdit_address").toString()))settings.setValue("lineEdit_address",ui->lineEdit_address->text());
        if(ui->lineEdit_credentials_folder->text()!=(settings.value("lineEdit_credentials_folder").toString()))settings.setValue("lineEdit_credentials_folder",ui->lineEdit_credentials_folder->text());
        if(ui->checkBox_keepconnected->isChecked()!=(settings.value("checkBox_keepconnected").toBool()))settings.setValue("checkBox_keepconnected",ui->checkBox_keepconnected->isChecked());

        textlistner->deleteLater();
        textlistner_thread->quit();
        textlistner_thread->wait();
        if(!textlistner_thread->isFinished())
        {
            qDebug()<<"worker thread still running we will now crash";
        }


     }
}

void MainWindow::toTray()
{
    hide();
    createTrayItem();
    trayIcon->show();
}

void MainWindow::createTrayItem()
{
    if(!trayIcon)
    {
        trayIcon=new QSystemTrayIcon(this);

       // trayIcon->setIcon(QIcon(":/images/icon.png"));

//        trayIcon->setIcon(QIcon("/home/jontio/qtprojects/LinuxNaturallySpeaking/Linux_Client/icon_small.png"));

        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this, SLOT(onTrayActivate(QSystemTrayIcon::ActivationReason)));

        QMenu *trayMenu = new QMenu(this);
        trayMenu->addAction("&Show", this, SLOT(onTrayShow()));
        trayMenu->addSeparator();
        trayMenu->addAction("E&xit", this, SLOT(onTrayExit()));
        trayIcon->setContextMenu(trayMenu);


        trayIcon->setToolTip(QCoreApplication::applicationName());


//        if(trayIcon)trayIcon->setIcon(QIcon(":/images/icon.png"));

       // trayIcon->setIcon(QIcon("/home/jontio/qtprojects/LinuxNaturallySpeaking/Linux_Client/icon_small2.png"));
        trayIcon->setIcon(QIcon("/home/jontio/qtprojects/LinuxNaturallySpeaking/Linux_Client/icon.png"));
        //trayIcon->setIcon(QIcon(":/images/icon.png"));

    }

}

void MainWindow::mousetimeout()
{
    QPoint globalCursorPos = QCursor::pos();
    qDebug()<<globalCursorPos.x()<<globalCursorPos.y();
}

MainWindow::~MainWindow()
{

    delete ui;
}

void MainWindow::connectToServerResult(bool connectOk)
{
    if(connectOk)
    {
        ui->pushButton->setEnabled(false);
        ui->pushButton_disconnect->setEnabled(true);
    }
     else
     {
        resetbuttonsstates();
     }
}

void MainWindow::setCredentialsResult(bool credentialsSetOk)
{
    if(credentialsSetOk)
    {
        QStringList strlist=ui->lineEdit_address->text().split(":");
        if(strlist.size()<2)
        {
            ui->textEditmain->append("need address with \":\" ie 0.0.0.0:12345");
            resetbuttonsstates();
        }
        else
        {
            QHostInfo res=QHostInfo::fromName(strlist[0]);
            if(res.addresses().size()==0)
            {
                ui->textEditmain->append("can't find address");
                resetbuttonsstates();
            }
            else
            {
                QHostAddress addr=res.addresses()[0];
                qDebug()<<addr.toString();
                emit connectToServer(addr.toString(),strlist[1].toInt());
            }

        }
    }
     else
     {
        ui->textEditmain->append("Can't open credential files: red_ca.pem, blue_local.pem and blue_local.key in folder \""+ui->lineEdit_credentials_folder->text()+"\"");
        resetbuttonsstates();
     }
}

void MainWindow::on_pushButton_clicked()
{
    if(!ui->pushButton->isEnabled())return;
    ui->pushButton->setEnabled(false);
    ui->pushButton_disconnect->setEnabled(false);

    QString cred_dir=ui->lineEdit_credentials_folder->text();
    emit setCredentials(cred_dir+"/red_ca.pem",cred_dir+"/blue_local.pem",cred_dir+"/blue_local.key");
}

void MainWindow::on_pushButton_disconnect_clicked()
{
    emit disconnectFromServer();
}

void MainWindow::resetbuttonsstates()
{
    ui->pushButton->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);
    qDebug()<<"buttons resetstate";
    if(ui->checkBox_keepconnected->isChecked())QTimer::singleShot(2000,this,SLOT(on_pushButton_clicked()));//keep trying to connect
}

void MainWindow::text(QString text)
{
    ui->textEditmain->append(text);
}

void MainWindow::text_voice(QString text)
{
    ui->textEditmain->append("Voice:"+text);

    bool iscontrolkey=false;
    QString controlkey;

    bool remove_last_string=false;

    bool clear_track_back=false;

    if(deadlinetimer.hasExpired())
    {

        if(text=="\b")text="backspace";

        QString text_simple=text.simplified().toLower();

        if(text_simple=="wake up"){audio_listening=true;return;}
        if(text_simple=="go to sleep"){audio_listening=false;return;}


        if(text_simple=="press enter")text="\r";
        if(text_simple=="go to start of line"){iscontrolkey=true;controlkey="Home";clear_track_back=true;}
        if(text_simple=="go to end of line"){iscontrolkey=true;controlkey="End";clear_track_back=true;}
        if(text_simple=="page up"){iscontrolkey=true;controlkey="Page_Up";clear_track_back=true;}
        if(text_simple=="page down"){iscontrolkey=true;controlkey="Page_Down";clear_track_back=true;}
        if(text_simple=="backspace"){iscontrolkey=true;controlkey="BackSpace";if(last_rx_text_stack.size())last_rx_text_stack[last_rx_text_stack.size()-1].chop(1);}
        if((text_simple=="scratch that")||(text_simple=="delete that")){text.clear();if(last_rx_text_stack.size())for(int i=0;i<last_rx_text_stack.last().size();i++)text+=QChar(8);remove_last_string=true;}

        if(text_simple=="music play")
        {
            QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
            QDBusInterface *interface = new QDBusInterface(QString("org.mpris.MediaPlayer2.")+"juk","/Player","org.kde.juk.player",dd,this);
            interface->call("stop");
            interface->call("play");
            return;
        }

        if(text_simple=="music stop")
        {
            QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
            QDBusInterface *interface = new QDBusInterface(QString("org.mpris.MediaPlayer2.")+"juk","/Player","org.kde.juk.player",dd,this);
            interface->call("stop");
            return;
        }

        if((text_simple=="music forward")||(text_simple.toLower()=="music ford"))
        {
            QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
            QDBusInterface *interface = new QDBusInterface(QString("org.mpris.MediaPlayer2.")+"juk","/Player","org.kde.juk.player",dd,this);
            interface->call("forward");
            return;
        }

        if(text_simple=="music back")
        {
            QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
            QDBusInterface *interface = new QDBusInterface(QString("org.mpris.MediaPlayer2.")+"juk","/Player","org.kde.juk.player",dd,this);
            interface->call("back");
            return;
        }

        if(text_simple.left(6)=="music ")
        {
            if(text_simple.mid(6,7)=="search ")
            {
                qDebug()<<"searching..."<<text_simple.mid(13);
                QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
                QDBusInterface *interface = new QDBusInterface(QString("org.mpris.MediaPlayer2.")+"juk","/Search","org.kde.juk.search",dd,this);
                if(text_simple.mid(13)=="clear")interface->call("setSearchText","");
                else interface->call("setSearchText",text_simple.mid(13));
                return;
            }
        }


    }
    deadlinetimer.setRemainingTime(1000);//require that there be one second off no talking between dictation and a command.

    if(!audio_listening)return;

    qDebug()<<last_rx_text_stack.size();

    xdo_t *mxdo=xdo_new(nullptr);
    Window window_ret;
    xdo_get_focused_window_sane((xdo_t*)mxdo,&window_ret);

    if(QCoreApplication::applicationPid()==xdo_get_pid_window((xdo_t*)mxdo,window_ret))
    {
        qDebug()<<"Focused window is us, not sending text to us.";
        xdo_free((xdo_t*)mxdo);
        return;
    }
    if(0==xdo_get_pid_window((xdo_t*)mxdo,window_ret))
    {
        qDebug()<<"Focused window has no pid, not sending text.";
        xdo_free((xdo_t*)mxdo);
        return;
    }

    unsigned char *name;
    int name_len_ret;
    int name_type;
    xdo_get_window_name((xdo_t*)mxdo,window_ret,&name,&name_len_ret,&name_type);
    QString windowname=QString::fromUtf8((char*)name,name_len_ret);
    XFree(name);

    if(!iscontrolkey)
    {
        bool process_sentence=true;

        if(text==".")process_sentence=false;
        if(text=="?")process_sentence=false;
        if(text=="!")process_sentence=false;
        if(text==",")process_sentence=false;
        if(text==";")process_sentence=false;
        if(text=="\"")process_sentence=false;//end quote
        if(text==")")process_sentence=false;//close parenthesis
        if(text=="]")process_sentence=false;//close square bracket
        if(text=="}")process_sentence=false;//close curly bracket

        if(text.trimmed().isEmpty())process_sentence=false;

        if(remove_last_string)process_sentence=false;

        if(process_sentence)
        {

            //get text from focused window??? well say it's allways this one
            QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
            QDBusInterface *interface = new QDBusInterface("org.zapto.jontio.natspeaking-"+QString::number(xdo_get_pid_window((xdo_t*)mxdo,window_ret)),"/","org.zapto.jontio.thing_for_screen_text",dd,this);
            QDBusReply<QStringList> reply=interface->call("getSurroundedText");
            if(reply.isValid())
            {
                qDebug()<<"return message="<<reply.value();
                QStringList strlist=reply.value();
                //text=text.trimmed();
                if((text.size()>0)&&(text[0]==" "))text.remove(0,1);
                if((strlist.size()>=3)&&(text.size()>0))
                {
                    if(strlist[0].isEmpty())text[0]=text[0].toUpper();
                    else
                    {
                        QString prestring=strlist[0];
                        QChar prestringstart=prestring[0];
                        QChar prestringend=prestring[prestring.size()-1];
                        int entercount=0;for(int i=0;i<prestring.size();i++)if(prestring[i]=='\n')entercount++;
                        if((prestringstart=='.')||(prestringstart=='?')||(prestringstart=='!')||(entercount>1))text[0]=text[0].toUpper();
                        if((prestringend!=' ')&&(prestringend!='\t')&&(prestringend!='\n')&&(prestringend!='\r')&&(prestringend!='"')&&(prestringend!='(')&&(prestringend!='<')&&(prestringend!='{')&&(prestringend!='[')&&(prestringend!='/')&&(prestringend!='-')&&(prestringend!='`')&&(prestringend!='\''))text.push_front(" ");

                    }
                    if(!strlist[2].isEmpty())
                    {
                        QString poststring=strlist[2];
                        QChar poststringstart=poststring[0];
                        QChar poststringend=poststring[poststring.size()-1];
                        Q_UNUSED(poststringend);
                        if((poststringstart!=' ')&&(poststringstart!='\t')&&(poststringstart!='\n')&&(poststringstart!='\r')&&(poststringstart!='.')&&(poststringstart!=',')&&(poststringstart!=';')&&(poststringstart!=')')&&(poststringstart!=']')&&(poststringstart!='}')&&(poststringstart!='?')&&(poststringstart!='!')&&(poststringstart!='/')&&(poststringstart!='>')&&(poststringstart!='"')&&(poststringstart!='`')&&(poststringstart!='\'')&&(poststringstart!='-'))text.push_back(" ");
                    }
                }
            } else qDebug()<<"Error: "<<reply.error().message();

        }


        if(remove_last_string&&last_rx_text_stack.size())last_rx_text_stack.removeLast();
        else if(!remove_last_string) last_rx_text_stack.push_back(text);
        while(last_rx_text_stack.size()>10)last_rx_text_stack.removeFirst();

        if(clear_track_back)last_rx_text_stack.clear();

        xdo_enter_text_window((xdo_t*)mxdo,CURRENTWINDOW,text.toUtf8().data(),120);
    }
    else xdo_send_keysequence_window((xdo_t*)mxdo,CURRENTWINDOW,controlkey.toLocal8Bit(),120);

    qDebug()<<last_rx_text_stack;

    xdo_free((xdo_t*)mxdo);
}


void MainWindow::on_checkBox_keepconnected_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    on_pushButton_clicked();
}
