#include "mdisubwindow.h"

#include <QTimer>
#include <QProgressDialog>
#include <QGuiApplication>

MdiSubWindow::MdiSubWindow(QWidget* _parent) : QMdiSubWindow(_parent), m_Timer(new QTimer(this)), m_Step(0)
{
    this->dlgProgress = new QProgressDialog(tr("Running, please wait..."), nullptr, 0, 80, this, Qt::WindowTitleHint | Qt::CustomizeWindowHint);
    this->dlgProgress->setWindowModality(Qt::WindowModal);
    this->dlgProgress->setCancelButton(nullptr);
    this->dlgProgress->setFixedSize(QSize(320, 80));

    connect(this, &MdiSubWindow::start, this, &MdiSubWindow::handleProgressStart);
    connect(this, &MdiSubWindow::stop, this, &MdiSubWindow::handleProgressStop);
    connect(this->m_Timer, &QTimer::timeout, this, &MdiSubWindow::handleProgressPerform);
}

MdiSubWindow::SubWindowType MdiSubWindow::subWindowType() const
{
    return MdiSubWindow::SubWindowType::SubWindow;
}

void MdiSubWindow::handleProgressStart()
{
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->m_Step = 0;
    this->m_Timer->start(50);
    //this->dlgProgress->show();
}

void MdiSubWindow::handleProgressStop()
{
    this->m_Step = 0;
    this->m_Timer->stop();
    this->dlgProgress->setValue(this->dlgProgress->maximum());
    QGuiApplication::restoreOverrideCursor();
}

void MdiSubWindow::handleProgressPerform()
{
    this->dlgProgress->setValue(m_Step++);
    if (m_Step == this->dlgProgress->maximum())
        m_Step = 0;
}
