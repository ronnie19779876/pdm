#ifndef MDISUBWINDOW_H
#define MDISUBWINDOW_H

#include <QMdiSubWindow>

class QProgressDialog;

class MdiSubWindow : public QMdiSubWindow
{
    Q_OBJECT

public:
    enum SubWindowType
    {
        SubWindow = 0x0,
        TopicSubWindow = 0x1,
        FunctionSubWindow = 0x2,
        PermissionSubWindow = 0x3,
        SinkSubWindow = 0x4,
        SourceSubWindow = 0x5,
    };
    Q_ENUM(SubWindowType)

    explicit MdiSubWindow(QWidget* parent = Q_NULLPTR);

    virtual MdiSubWindow::SubWindowType subWindowType() const;

signals:
    void completeWindowActivated();
    void start();
    void stop();

private:
    QProgressDialog* dlgProgress;
    QTimer* m_Timer;
    int m_Step;

private slots:
    void handleProgressStart();
    void handleProgressStop();
    void handleProgressPerform();
};

#endif // MDISUBWINDOW_H
