#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>
#include <QtDBus>
#include <QDBusConnection>
#include <QHostInfo>
#include <QHostAddress>

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

    ui->pushButton->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);

    ui->textEditmain->append("started");
    textlistner=new TextListner(this);
    connect(textlistner,SIGNAL(text(QString)),this,SLOT(text(QString)));
    connect(textlistner,SIGNAL(text_voice(QString)),this,SLOT(text_voice(QString)));
    connect(textlistner,SIGNAL(disconnected()),this,SLOT(resetbuttonsstates()));

    qDebug()<<"my window id"<<QCoreApplication::applicationPid();

    QTimer *timer=new QTimer(this);
    connect(timer,SIGNAL(timeout()),this,SLOT(mousetimeout()));

    on_pushButton_clicked();
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

void MainWindow::on_pushButton_clicked()
{

    QStringList strlist=ui->lineEdit_address->text().split(":");
    if(strlist.size()<2)
    {
        ui->textEditmain->append("need address with \":\" ie 0.0.0.0:12345");
    }
    else
    {
        QHostInfo res=QHostInfo::fromName(strlist[0]);
        if(res.addresses().size()==0)
        {
            ui->textEditmain->append("can't find address");
        }
        else
        {
            QHostAddress addr=res.addresses()[0];
            qDebug()<<addr.toString();
            if(textlistner->connectToServer(addr.toString(),strlist[1].toInt()))
            {
                ui->pushButton->setEnabled(false);
                ui->pushButton_disconnect->setEnabled(true);
            }
        }

    }



}

void MainWindow::on_pushButton_disconnect_clicked()
{
    textlistner->disconnectFromServer();
    resetbuttonsstates();

}

void MainWindow::resetbuttonsstates()
{
    ui->pushButton->setEnabled(true);
    ui->pushButton_disconnect->setEnabled(false);
    qDebug()<<"buttons resetstate";
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

