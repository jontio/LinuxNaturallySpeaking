#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "textlistner.h"
#include <QElapsedTimer>
#include <QDeadlineTimer>
#include "dbusobj.h"
#include <QSystemTrayIcon>
#include <QCloseEvent>
#include <QPointer>

#define AppSessionName "main_app_session"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *event);
public slots:
    void CloseAccept();
private slots:
    void on_pushButton_clicked();
    void on_pushButton_disconnect_clicked();
    void text(QString text);
    void text_voice(QString text);
    void mousetimeout();
    void resetbuttonsstates();
    void createTrayItem();
    void onTrayExit();
    void onTrayShow();
    void onTrayActivate(QSystemTrayIcon::ActivationReason reason);
    void toTray();
    void connectToServerResult(bool connectOk);
    void setCredentialsResult(bool credentialsSetOk);
    void on_checkBox_keepconnected_stateChanged(int arg1);

signals:
    void connectToServer(QString hostname,qint16 port);
    void setCredentials(QString CaRemote,QString CaLocal,QString KeyPrivate);
    void disconnectFromServer();
private:
    Ui::MainWindow *ui;
    TextListner *textlistner;
    QStringList last_rx_text_stack;
    bool audio_listening;
    QElapsedTimer time_elapsed;
    QDeadlineTimer deadlinetimer;
    DbusObj *dbusobj;
    QPointer<QSystemTrayIcon> trayIcon;
    bool ignore_close;

    QThread *textlistner_thread;

};

#endif // MAINWINDOW_H
