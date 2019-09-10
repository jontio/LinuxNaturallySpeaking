// Avoid multiple header inclusion
#ifndef TEXTVIEW_H
#define TEXTVIEW_H


#include <QString>
#include <QObject>

#include <KTextEditor/Plugin>
#include <ktexteditor/mainwindow.h>
#include <ktexteditor/configpage.h>

#include <QList>
#include <QKeyEvent>

#include <KXMLGUIClient>

#include <ktexteditor/view.h>
#include <ktexteditor/cursor.h>

#include <QDBusConnection>
#include <QDBusAbstractAdaptor>
#include <QPointer>

#define AppSessionName "main_app_session"

class DbusObj : public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.zapto.jontio.thing_for_screen_text")
public:
    explicit DbusObj(QObject *parent = nullptr);
    QPointer<KTextEditor::View> view;
Q_SIGNALS:
public Q_SLOTS:
    QString getText();
    QString getExtendedSelectionText();
    QStringList getSurroundedText();
private:
};


class TextViewPlugin : public KTextEditor::Plugin
{

  public:

    explicit TextViewPlugin( QObject* parent = nullptr, const QList<QVariant>& = QList<QVariant>() );
    virtual ~TextViewPlugin();

    QObject *createView (KTextEditor::MainWindow *mainWindow) override;

    int configPages() const override { return 0; }

    KTextEditor::ConfigPage *configPage (int number = 0, QWidget *parent = nullptr) override;

    void readConfig();
    DbusObj *dbusobj;
    qint64 apppid;
  private:



};



class TextViewPluginView : public QObject
{
  Q_OBJECT
public:
    TextViewPluginView(TextViewPlugin* plugin, KTextEditor::MainWindow *mainWindow);
    ~TextViewPluginView() override;
public Q_SLOTS:
    void textInserted(KTextEditor::View *view,const KTextEditor::Cursor &position,const QString &text);
    void viewChanged(KTextEditor::View *view);
    void cursorPositionChanged(KTextEditor::View *view, const KTextEditor::Cursor &newPosition);
    void selectionChanged (KTextEditor::View *view);
private:
    TextViewPlugin *m_plugin;
    QPointer<KTextEditor::View> last_view;
    QPointer<DbusObj> dbusobj;
};



#endif // TEXTVIEW_H 
