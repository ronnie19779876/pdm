#include "sinkswindow.h"

#include <QTableWidget>
#include <QMenu>
#include <QMessageBox>

#include "../services/sinkservice.h"
#include "../widgets/functioninstanceswindow.h"
#include "../widgets/sinkinputwindow.h"

SinksWindow::SinksWindow(QWidget* parent) : BaseMdiSubWindow(parent), m_SinkService(new SinkService(this))
{
    m_Columns << "tenant" << "namesapces" << "name"  << "inputs" << "instanceNum" << "runningNum";

    QStringList header;
    header << tr("Tenant") << tr("Namespace") << tr("Name") << tr("inputs") << tr("Instances") << tr("Running");
    this->twTable->setColumnCount(header.length());
    this->twTable->setHorizontalHeaderLabels(header);

    setWindowTitle(tr("Sinks"));
    setWindowIcon(QIcon(":/export"));

    connect(this, &SinksWindow::initialize, this, &SinksWindow::handleReload);
    connect(this->twTable, &QTableWidget::itemDoubleClicked, this, &SinksWindow::handleTableItemDoubleClicked);
}

SinksWindow::~SinksWindow() {}

MdiSubWindow::SubWindowType SinksWindow::subWindowType() const
{
    return MdiSubWindow::SubWindowType::SinkSubWindow;
}

void SinksWindow::afterWindowActivated(const QVariant& _var)
{
    BaseMdiSubWindow::afterWindowActivated(_var);

    if (!this->m_HasCreateActions)
    {
        createActions();
        createToolbar();
        this->m_HasCreateActions = true;
    }

    emit initialize();
}

void SinksWindow::createActions()
{
    BaseMdiSubWindow::createActions();

    this->actNew->setText(tr("&Create Sink"));
    this->actNew->setStatusTip(tr("Creates a new Pulsar Sink in cluster mode."));

    this->actStart->setText(tr("&Start Sink"));
    this->actStart->setStatusTip(tr("Start all instances of a Pulsar Sink."));

    this->actStop->setText(tr("&Stop Sink"));
    this->actStop->setStatusTip(tr("Stop all instances of a Pulsar Sink."));

    this->actDelete->setText(tr("&Delete Sink"));
    this->actDelete->setStatusTip(tr("Deletes a Pulsar Sink currently running in cluster mode."));

    this->actRefresh->setText(tr("&Refresh Sinks"));
    this->actRefresh->setStatusTip(tr("Refresh Sinks."));

    connect(this->actNew, &QAction::triggered, this, &SinksWindow::handleNewSink);
    connect(this->actRefresh, &QAction::triggered, this, &SinksWindow::handleReload);

    this->actUpdate->setText(tr("&Update Sink..."));
    this->actUpdate->setStatusTip(tr("Updates a Pulsar Sink currently running in cluster mode."));

    this->actInstances->setText(tr("&Sink instances..."));

    connect(this->actInstances, &QAction::triggered, this, &SinksWindow::handleSinkInstances);
    connect(this->actUpdate, &QAction::triggered, this, &SinksWindow::handleUpdateSink);
}

void SinksWindow::handleNewSink(bool)
{
    BaseInputWindow* win = new SinkInputWindow;
    connect(this, &SinksWindow::activateNewSinkWindow, win, &BaseInputWindow::afterWindowActivated);
    connect(win, &BaseInputWindow::completedInput, this, &SinksWindow::handleInsertNewSink);
    emit activateNewSinkWindow(this->variant());
    win->exec();
}

void SinksWindow::handleUpdateSink(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current != Q_NULLPTR)
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Sink>())
        {
            Sink sink = data.value<Sink>();
            BaseInputWindow* win = new SinkInputWindow;
            connect(this, &SinksWindow::activateUpdateSinkWindow, win, &BaseInputWindow::afterWindowActivated);
            emit activateUpdateSinkWindow(QVariant::fromValue(sink));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void SinksWindow::handleReload()
{
    emit start();

    this->twTable->clearContents();
    Namespace ns = value<Namespace>();
    QList<Sink> sinks = this->m_SinkService->sinks(ns);
    this->twTable->setRowCount(sinks.length());
    for (int i = 0, n = sinks.length(); i < n ; i++)
    {
        SinkData data = sinks[i].toData();
        for (int j = 0, m = m_Columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[j]]);
            this->twTable->setItem(i, j, item);

            if (j > 1 && j < 4)
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));

            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(sinks[i]));
        }
    }

    emit stop();
}

void SinksWindow::handleInsertNewSink(const QVariant& _var)
{
    if (_var.canConvert<Sink>())
    {
        Sink sink = _var.value<Sink>();
        int row = this->twTable->rowCount();
        this->twTable->insertRow(++row);
        this->twTable->setRowCount(row);

        SinkData data = sink.toData();
        for (int i = 0, n = m_Columns.length(); i < n; i++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[i]]);
            this->twTable->setItem(row - 1, i, item);

            if (i > 1 && i < 4)
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));

            if (i == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(sink));
        }
    }
}

void SinksWindow::handleSinkInstances(bool)
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

void SinksWindow::handleTableItemDoubleClicked(QTableWidgetItem* _item)
{
    int row = _item->row();
    QTableWidgetItem* item = this->twTable->item(row, 0);
    QVariant data = item->data(Qt::UserRole);
    if (data.canConvert<Sink>())
    {
        FunctionInstancesWindow* win = new FunctionInstancesWindow;
        connect(this, &SinksWindow::activateSinkInstanceWindow, win, &FunctionInstancesWindow::afterWindowActivated);
        emit activateSinkInstanceWindow(data);
        win->exec();
    }
}

bool SinksWindow::doDelete(const QVariant& _data,  HttpStatusCode& _error)
{
    if (_data.canConvert<Sink>())
    {
        Sink sink = _data.value<Sink>();
        this->m_SinkService->remove(sink, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool SinksWindow::doStart(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Sink>())
    {
        Sink sink = _data.value<Sink>();
        this->m_SinkService->start(sink, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool SinksWindow::doStop(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Sink>())
    {
        Sink sink = _data.value<Sink>();
        this->m_SinkService->stop(sink, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}
