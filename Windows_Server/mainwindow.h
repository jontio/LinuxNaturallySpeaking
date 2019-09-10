#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QTimer>
#include "textserver.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


public slots:
    void keypressed(QKeyEvent *e);
    void rx_from_client(QTcpSocket *clientSocket);
    void info(const QString &text);

    void keypaste();
private slots:
    void sendtext_to_clients();
    void on_checkBox_enable_clicked();

private:
    Ui::MainWindow *ui;
    TextServer *textserver;
    QString text_for_clients;
    QTimer *text_for_clients_timer;

};

#endif // MAINWINDOW_H
