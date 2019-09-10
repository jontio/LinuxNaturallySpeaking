#include "mytextedit.h"
#include <QKeyEvent>
#include <QDebug>

mytextedit::mytextedit(QWidget *parent) : QTextEdit(parent)
{
    passthrough=false;
}

void mytextedit::keyPressEvent(QKeyEvent *e)
{

    if(!passthrough)emit keypressed(e);
    passthrough=false;

//    return;
//    if(!passthrough)
//    {
//        emit keypressed(e);
//        return;
//    }
//    qDebug()<<e->type();
//    qDebug()<<e->modifiers();
//    qDebug()<<e->key();
//    QTextEdit::keyPressEvent(e);

}


