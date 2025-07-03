#include "sourceswindow.h"

#include <QTableWidget>
#include <QVBoxLayout>
#include <QToolBar>
#include <QHeaderView>
#include <QMessageBox>
#include <QMenu>
#include <QApplication>

#include "../sink.h"
#include "../services/sourceservice.h"
#include "../widgets/sourceinputwindow.h"
#include "../widgets/functioninstanceswindow.h"

SourcesWindow::SourcesWindow(QWidget* parent) : BaseMdiSubWindow(parent), m_SourceService(new SourceService(this))
{
    QStringList header;
    header << tr("Tenant") << tr("Namespace") << tr("Name") << tr("TopicName") << tr("ClassName") << tr("Instances") << tr("Running");
    this->twTable->setColumnCount(header.length());
    this->twTable->setHorizontalHeaderLabels(header);

    setWindowTitle(tr("Sources"));
    setWindowIcon(QIcon(":/import"));

    connect(this, &SourcesWindow::initialize, this, &SourcesWindow::handleReload);
    connect(this, &SourcesWindow::completeStart, this, &SourcesWindow::handleReload);
    connect(this, &SourcesWindow::completeStop, this, &SourcesWindow::handleReload);
    connect(this->twTable, &QTableWidget::itemDoubleClicked, this, &SourcesWindow::handleTableItemDoubleClicked);
}

MdiSubWindow::SubWindowType SourcesWindow::subWindowType() const
{
    return MdiSubWindow::SubWindowType::SourceSubWindow;
}

void SourcesWindow::afterWindowActivated(const QVariant& _var)
{
    BaseMdiSubWindow::afterWindowActivated(_var);

    if (!this->m_HasCreateActions)
    {
        createActions();
        createToolbar();
        this->m_HasCreateActions = true;
    }
    this->showMaximized();
    emit initialize();
}

void SourcesWindow::handleReload()
{
    emit start();
    QStringList columns;
    columns << "tenant" << "namesapces" << "name"  << "topicName" << "className" << "instanceNum" << "runningNum";
    this->twTable->clearContents();
    Namespace ns = value<Namespace>();
    QList<Source> sources = this->m_SourceService->sources(ns);
    this->twTable->setRowCount(sources.length());
    for (int i = 0, n = sources.length(); i < n ; i++)
    {
        SinkData data = sources[i].toData();
        for (int j = 0, m = columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
            this->twTable->setItem(i, j, item);

            if (j > 1 && j < 5)
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));

            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(sources[i]));
        }
    }
    emit stop();
}

void SourcesWindow::handleNewSource(bool)
{
    BaseInputWindow* win = new SourceInputWindow;
    connect(this, &SourcesWindow::activateNewSourceWindow, win, &BaseInputWindow::afterWindowActivated);
    connect(win, &BaseInputWindow::completedInput, this, &SourcesWindow::handleInsertNewSource);
    emit activateNewSourceWindow(this->variant());
    win->exec();
}

void SourcesWindow::handleUpdateSource(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current != Q_NULLPTR)
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Source>())
        {
            Source source = data.value<Source>();
            BaseInputWindow* win = new SourceInputWindow;
            connect(this, &SourcesWindow::activateUpdateSourceWindow, win, &BaseInputWindow::afterWindowActivated);
            emit activateUpdateSourceWindow(QVariant::fromValue(source));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void SourcesWindow::handleSourceInstances()
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current != Q_NULLPTR)
    {
        handleTableItemDoubleClicked(current);
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void SourcesWindow::handleInsertNewSource(const QVariant& _var)
{
    if (_var.canConvert<Source>())
    {
        Source source = _var.value<Source>();
        int row = this->twTable->rowCount();
        this->twTable->insertRow(++row);
        this->twTable->setRowCount(row);
        QStringList columns;
        columns << "tenant" << "namesapces" << "name"  << "topicName" << "className" << "instanceNum";
        SourceData data = source.toData();
        for (int i = 0, n = columns.length(); i < n; i++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[columns[i]]);
            this->twTable->setItem(row - 1, i, item);

            if (i > 1 && i < 5)
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));

            if (i == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(source));
        }
    }
}

void SourcesWindow::handleTableItemDoubleClicked(QTableWidgetItem* _item)
{
    int row = _item->row();
    QTableWidgetItem* item = this->twTable->item(row, 0);
    QVariant data = item->data(Qt::UserRole);
    if (data.canConvert<Source>())
    {
        FunctionInstancesWindow* win = new FunctionInstancesWindow;
        connect(this, &SourcesWindow::activateSourceInstanceWindow, win, &FunctionInstancesWindow::afterWindowActivated);
        emit activateSourceInstanceWindow(data);
        win->exec();
    }
}

void SourcesWindow::createActions()
{
    BaseMdiSubWindow::createActions();

    //toolbar actions
    this->actNew->setText(tr("&Create Source"));
    this->actNew->setStatusTip(tr("Creates a new Pulsar Source in cluster mode."));

    this->actStart->setText(tr("&Start Source"));
    this->actStart->setStatusTip(tr("Start all instances of a Pulsar Source."));

    this->actStop->setText(tr("&Stop Source"));
    this->actStop->setStatusTip(tr("Stop all instances of a Pulsar Source."));

    this->actDelete->setText(tr("&Delete Source"));
    this->actDelete->setStatusTip(tr("Deletes a Pulsar Sink currently running in cluster mode."));

    this->actRefresh->setText(tr("&Refresh Sources"));
    this->actRefresh->setStatusTip(tr("Refresh Sources."));

    connect(this->actNew, &QAction::triggered, this, &SourcesWindow::handleNewSource);
    connect(this->actRefresh, &QAction::triggered, this, &SourcesWindow::handleReload);

    //popup menu actions
    this->actUpdate->setText(tr("&Update Source..."));
    this->actUpdate->setStatusTip(tr("Updates a Pulsar Source currently running in cluster mode."));

    this->actInstances->setText(tr("&Source instances..."));

    connect(this->actInstances, &QAction::triggered, this, &SourcesWindow::handleSourceInstances);
    connect(this->actUpdate, &QAction::triggered, this, &SourcesWindow::handleUpdateSource);
}

bool SourcesWindow::doDelete(const QVariant& _data,  HttpStatusCode& _error)
{
    if (_data.canConvert<Source>())
    {
        Source source = _data.value<Source>();
        this->m_SourceService->deleteSource(source, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool SourcesWindow::doStart(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Source>())
    {
        Source source = _data.value<Source>();
        this->m_SourceService->startSource(source, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool SourcesWindow::doStop(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Source>())
    {
        Source source = _data.value<Source>();
        this->m_SourceService->stopSource(source, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}
