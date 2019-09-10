#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "textlistner.h"
#include <QElapsedTimer>
#include <QDeadlineTimer>
#include "dbusobj.h"

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


private slots:
    void on_pushButton_clicked();
    void on_pushButton_disconnect_clicked();
    void text(QString text);
    void text_voice(QString text);
    void mousetimeout();
    void resetbuttonsstates();
private:
    Ui::MainWindow *ui;
    TextListner *textlistner;
    QStringList last_rx_text_stack;
    bool audio_listening;
    QElapsedTimer time_elapsed;
    QDeadlineTimer deadlinetimer;
    DbusObj *dbusobj;
};

#endif // MAINWINDOW_H
