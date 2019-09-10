
#include "textview.h"

#include <ktexteditor/document.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>

#include <kdatetime.h>

#include <ktexteditor/range.h>
#include <ktexteditor/movingcursor.h>
#include <ktexteditor/movinginterface.h>

#include <QtDBus>
#include <QDBusConnection>
#include <QApplication>


K_PLUGIN_FACTORY_WITH_JSON(TextViewPluginFactory,"ktexteditor_textview.json", registerPlugin<TextViewPlugin>();)

// Constructor

TextViewPlugin::TextViewPlugin( QObject* parent, const QList<QVariant>& ):
    KTextEditor::Plugin ( parent )
{
    qDebug()<<"TextViewPlugin::TextViewPlugin";

    dbusobj=new DbusObj(this);
    QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);

    apppid=QApplication::applicationPid();

    // qDebug()<<"conn";
    qDebug()<<dd.name();
    dd.registerObject("/",dbusobj,QDBusConnection::ExportAllSlots);
    dd.registerService("org.zapto.jontio.natspeaking-"+QString::number(apppid));

}

// Destructor
TextViewPlugin::~TextViewPlugin()
{
    QDBusConnection dd=QDBusConnection::connectToBus(QDBusConnection::SessionBus,AppSessionName);
    dd.unregisterObject("/");
    dd.unregisterService("org.zapto.jontio.natspeaking-"+QString::number(apppid));

    qDebug()<<"TextViewPlugin::~TextViewPlugin";
}

QObject *TextViewPlugin::createView(KTextEditor::MainWindow *mainWindow)
{
    qDebug()<<"TextViewPlugin::createView";
    TextViewPluginView *view = new TextViewPluginView (this, mainWindow);

    qDebug()<<"TextViewPlugin::createView view="<<view;
    return view;
}

KTextEditor::ConfigPage *TextViewPlugin::configPage(int number, QWidget *parent)
{
    Q_UNUSED(number);
    Q_UNUSED(parent);
    qDebug()<<"configPage";
    return nullptr;
}

void TextViewPlugin::readConfig()
{

}


// Plugin view class
TextViewPluginView::TextViewPluginView(TextViewPlugin* plugin, KTextEditor::MainWindow *mainWindow)
    : QObject(mainWindow),m_plugin(plugin)
{
    qDebug()<<"TextViewPluginView::TextViewPluginView";

    dbusobj=plugin->dbusobj;
    connect(mainWindow, SIGNAL(viewChanged(KTextEditor::View *)),this,SLOT(viewChanged(KTextEditor::View *)));



}

// Destructor
TextViewPluginView::~TextViewPluginView()
{
    qDebug()<<"TextViewPluginView::~TextViewPluginView";
}


void TextViewPluginView::textInserted(KTextEditor::View *view,const KTextEditor::Cursor &position,const QString &text)
{
    Q_UNUSED(view);
    Q_UNUSED(position);
    Q_UNUSED(text);
    //qDebug()<<"textInsertedview"<<position.line()<<position.column()<<text;
    //KTextEditor::Range range(position,4);
    //qDebug()<<"dat=="<<view->document()->text(range);
}

void TextViewPluginView::cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &newPosition)
{
    Q_UNUSED(view);
    Q_UNUSED(newPosition);
//     qDebug()<<"cursorPositionChanged"<<newPosition.line()<<newPosition.column();
// 
//     KTextEditor::MovingInterface *moving=qobject_cast<KTextEditor::MovingInterface*>(view->document());
//     KTextEditor::MovingCursor *cursor=moving->newMovingCursor(newPosition);
//     cursor->move(-1);
// 
//     KTextEditor::Range range(cursor->toCursor(),4);
//     qDebug()<<"-1 to +3 =="<<view->document()->text(range);
//     qDebug()<<"word =="<<view->document()->wordAt(newPosition);
//     qDebug()<<"line =="<<view->document()->line(newPosition.line());
}

void TextViewPluginView::selectionChanged(KTextEditor::View *view)
{
    qDebug()<<"selectionChanged";
    if(!view->selection())
    {
        qDebug()<<"no selection";
        return;
    }

    qDebug()<<"selection =="<<view->selectionText();


    KTextEditor::MovingInterface *moving=qobject_cast<KTextEditor::MovingInterface*>(view->document());
    KTextEditor::MovingCursor *cursor_start=moving->newMovingCursor(view->selectionRange().start());
    cursor_start->move(-1);
    KTextEditor::MovingCursor *cursor_end=moving->newMovingCursor(view->selectionRange().end());
    cursor_end->move(1);

    KTextEditor::Range range(cursor_start->toCursor(),cursor_end->toCursor());

    qDebug()<<"selection -1 to +1 =="<<view->document()->text(range);

}

void TextViewPluginView::viewChanged(KTextEditor::View *view)
{
    qDebug()<<"TextViewPluginView::viewChanged";

    if(last_view)
    {
        qDebug()<<"exists";
        disconnect(last_view, SIGNAL(
                       textInserted(KTextEditor::View *,const KTextEditor::Cursor &,const QString &)
                       ), this, SLOT(
                       textInserted(KTextEditor::View *,const KTextEditor::Cursor &,const QString &)
                       )
                   );
        disconnect(last_view, SIGNAL(
                       cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)
                       ), this, SLOT(
                       cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)
                       )
                   );
        disconnect(last_view, SIGNAL(
                       selectionChanged(KTextEditor::View *)
                       ), this, SLOT(
                       selectionChanged(KTextEditor::View *)
                       )
                   );
    } else qDebug()<<"null";
    connect(view, SIGNAL(
                textInserted(KTextEditor::View *,const KTextEditor::Cursor &,const QString &)
                ), this, SLOT(
                textInserted(KTextEditor::View *,const KTextEditor::Cursor &,const QString &)
                )
            );
    connect(view, SIGNAL(
                cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)
                ), this, SLOT(
                cursorPositionChanged(KTextEditor::View *, const KTextEditor::Cursor &)
                )
            );
    connect(view, SIGNAL(
                selectionChanged(KTextEditor::View *)
                ), this, SLOT(
                selectionChanged(KTextEditor::View *)
                )
            );

    last_view=view;
    dbusobj->view=view;
}

DbusObj::DbusObj(QObject *parent) : QDBusAbstractAdaptor(parent)
{

}

QString DbusObj::getText()
{
    qDebug()<<"getText";
    if(!view)return "";
    return view->document()->text();
}

QString DbusObj::getExtendedSelectionText()
{
    qDebug()<<"getExtendedSelectionText";
    if(!view)return "";
    if(!view->selection())
    {
        return "";
    }
    KTextEditor::MovingInterface *moving=qobject_cast<KTextEditor::MovingInterface*>(view->document());
    KTextEditor::MovingCursor *cursor_start=moving->newMovingCursor(view->selectionRange().start());
    cursor_start->move(-1);
    KTextEditor::MovingCursor *cursor_end=moving->newMovingCursor(view->selectionRange().end());
    cursor_end->move(1);

    KTextEditor::Range range(cursor_start->toCursor(),cursor_end->toCursor());

    return view->document()->text(range);
}

QStringList DbusObj::getSurroundedText()
{
    KTextEditor::MovingInterface *moving=qobject_cast<KTextEditor::MovingInterface*>(view->document());
    KTextEditor::MovingCursor *cursor=moving->newMovingCursor(view->cursorPosition());

    KTextEditor::Cursor startofselection=view->cursorPosition();
    KTextEditor::Cursor endofselection=startofselection;
    if(view->selection())
    {
        startofselection=view->selectionRange().start();
        endofselection=view->selectionRange().end();
        cursor->setPosition(startofselection);
    }

    QStringList strlist;


    bool haspreviousword=false;
    while(cursor->move(-1))
    {
        QChar ch=view->document()->characterAt(cursor->toCursor());
        if(ch.toLatin1()==0)continue;//a line marker
        if(!ch.isSpace())
        {
            haspreviousword=true;
            break;
        }
    }
    qDebug()<<haspreviousword;
    KTextEditor::Range range;
    range.setStart(cursor->toCursor());
    range.setEnd(startofselection);

    strlist<<view->document()->text(range);//from last non whitespace to cursor/start of selection



    qDebug()<<"b4="<<view->document()->text(range);

    QString wordb4;
    if(haspreviousword)
    {
        cursor->move(1);
        range.setEnd(cursor->toCursor());
        while(cursor->move(-1))
        {
            QChar ch=view->document()->characterAt(cursor->toCursor());
            if(ch.isSpace()||(ch.toLatin1()==0))
            {
                cursor->move(1);
                break;
            }
        }
        range.setStart(cursor->toCursor());
        wordb4=view->document()->text(range);
    }

    //selection
    if(view->selection())
    {
        qDebug()<<"selection="<<view->document()->text(view->selectionRange());
        strlist<<view->document()->text(view->selectionRange());
    } else strlist<<"";

    bool hasnextword=false;
    cursor->setPosition(endofselection);
    while(cursor->move(1))
    {
        QChar ch=view->document()->characterAt(cursor->toCursor());
        if(ch.toLatin1()==0)continue;//a line marker
        if(!ch.isSpace())
        {
            hasnextword=true;
            break;
        }
    }
    qDebug()<<hasnextword;
    range.setStart(endofselection);
    range.setEnd(cursor->toCursor());

    strlist<<view->document()->text(range);
    qDebug()<<"after="<<view->document()->text(range);

    //word b4
    strlist<<wordb4;
    qDebug()<<"wordb4="<<wordb4;


    return strlist;

}


// We need to include the moc file since we have declared slots and we are using
// the Q_OBJECT macro on the TextViewPluginView class.
#include "textview.moc"
