#ifndef MYTEXTEDIT_H
#define MYTEXTEDIT_H

#include <QObject>
#include <QTextEdit>
#include <QChar>

class mytextedit : public QTextEdit
{
    Q_OBJECT
public:
    mytextedit(QWidget *parent = nullptr);
    bool passthrough;
protected:
    void keyPressEvent(QKeyEvent *e);
signals:
    void keypressed(QKeyEvent *e);
};

#endif // MYTEXTEDIT_H
