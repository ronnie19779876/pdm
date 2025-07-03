#include "functionswindow.h"

#include <QVBoxLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>

#include "../services/functionservice.h"
#include "../widgets/newfunctionwindow.h"
#include "../widgets/functioninstanceswindow.h"

FunctionsWindow::FunctionsWindow(QWidget* parent) : BaseMdiSubWindow(parent), m_FunctionService(new FunctionService(this))
{
    m_Header << tr("Tenant") << tr("Namespace") << tr("Name") << tr("Instances") << tr("Running");
    m_Columns  << "tenant" << "namesapces" << "name"  << "instances" << "running";

    this->twTable->setColumnCount(m_Header.length());
    this->twTable->setHorizontalHeaderLabels(m_Header);

    setWindowTitle(tr("Functions"));
    setWindowIcon(QIcon(":/function"));

    connect(this, &FunctionsWindow::initialize, this, &FunctionsWindow::handleReload);
    connect(this->twTable, &QTableWidget::itemDoubleClicked, this, &FunctionsWindow::handleTableItemDoubleClicked);
}

MdiSubWindow::SubWindowType FunctionsWindow::subWindowType() const
{
    return MdiSubWindow::SubWindowType::FunctionSubWindow;
}

void FunctionsWindow::afterWindowActivated(const QVariant& _var)
{
    BaseMdiSubWindow::afterWindowActivated(_var);

    if (!m_HasCreateActions)
    {
        createActions();
        createToolbar();
        this->m_HasCreateActions = true;
    }

    emit initialize();
    showMaximized();
}

void FunctionsWindow::createActions()
{
    BaseMdiSubWindow::createActions();

    this->actNew->setText(tr("&Create Function"));
    this->actNew->setStatusTip(tr("Creates a new Pulsar Function in cluster mode."));

    this->actStart->setText(tr("&Start Function"));
    this->actStart->setStatusTip(tr("Start all instances of a Pulsar Function."));

    this->actStop->setText(tr("&Stop Function"));
    this->actStop->setStatusTip(tr("Stop all instances of a Pulsar Function."));

    this->actDelete->setText(tr("&Delete Function"));
    this->actDelete->setStatusTip(tr("Deletes a Pulsar Function currently running in cluster mode."));

    this->actRefresh->setText(tr("&Refresh Functions"));
    this->actRefresh->setStatusTip(tr("Refresh Functions."));

    connect(this->actNew, &QAction::triggered, this, &FunctionsWindow::handleNewFunction);
    connect(this->actRefresh, &QAction::triggered, this, &FunctionsWindow::handleReload);

    this->actUpdate->setText(tr("&Update Function..."));
    this->actUpdate->setStatusTip(tr("Updates a Pulsar Function currently running in cluster mode."));

    this->actInstances->setText(tr("&Function instances..."));

    connect(this->actInstances, &QAction::triggered, this, &FunctionsWindow::handleFunctionInstances);
    connect(this->actUpdate, &QAction::triggered, this, &FunctionsWindow::handleUpdateFunction);
    //Log4Qt::Logger::logger("main")->debug("FunctionsWindow::createActions()");
}

bool FunctionsWindow::doDelete(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Function>())
    {
        Function function = _data.value<Function>();
        this->m_FunctionService->deleteFunction(function, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool FunctionsWindow::doStart(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Function>())
    {
        Function function = _data.value<Function>();
        this->m_FunctionService->startFunction(function, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

bool FunctionsWindow::doStop(const QVariant& _data, HttpStatusCode& _error)
{
    if (_data.canConvert<Function>())
    {
        Function function = _data.value<Function>();
        this->m_FunctionService->stopFunction(function, _error);
        if (_error.code == HttpStatusCode::StatusCode::NoContent)
        {
            return true;
        }
    }
    return false;
}

void FunctionsWindow::handleNewFunction(bool)
{
    BaseInputWindow* win = new NewFunctionWindow;
    connect(this, &FunctionsWindow::activateNewFunctionWindow, win, &BaseInputWindow::afterWindowActivated);
    connect(win, &BaseInputWindow::completedInput, this, &FunctionsWindow::handleInsertNewFunction);
    emit activateNewFunctionWindow(this->variant());
    win->exec();
}

void FunctionsWindow::handleUpdateFunction(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current != Q_NULLPTR)
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Function>())
        {
            Function function = data.value<Function>();
            BaseInputWindow* win = new NewFunctionWindow;
            connect(this, &FunctionsWindow::activateUpdateFunctionWindow, win, &BaseInputWindow::afterWindowActivated);
            emit activateUpdateFunctionWindow(QVariant::fromValue(function));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void FunctionsWindow::handleReload()
{
    emit start();

    this->twTable->clearContents();
    Namespace ns = value<Namespace>();
    QList<Function> functions = this->m_FunctionService->functions(ns);
    this->twTable->setRowCount(functions.length());
    for (int i = 0, n = functions.length(); i < n ; i++)
    {
        FunctionData data = functions[i].toData();
        for (int j = 0, m = m_Columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[j]]);
            this->twTable->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(functions[i]));
        }
    }

    emit stop();
}

void FunctionsWindow::handleInsertNewFunction(const QVariant& _var)
{
    if (_var.canConvert<Function>())
    {
        Function function = _var.value<Function>();
        int row = this->twTable->rowCount();
        this->twTable->insertRow(++row);
        this->twTable->setRowCount(row);

        FunctionData data = function.toData();
        for (int i = 0, n = m_Columns.length(); i < n; i++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[i]]);
            this->twTable->setItem(row - 1, i, item);

            if (i == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(function));
        }
    }
}

void FunctionsWindow::handleFunctionInstances(bool)
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

void FunctionsWindow::handleTableItemDoubleClicked(QTableWidgetItem* _item)
{
    int row = _item->row();
    QTableWidgetItem* first = this->twTable->item(row, 0);
    QVariant data = first->data(Qt::UserRole);
    if (data.canConvert<Function>())
    {
        FunctionInstancesWindow* win = new FunctionInstancesWindow;
        connect(this, &FunctionsWindow::activateFunctionInstanceWindow, win, &FunctionInstancesWindow::afterWindowActivated);
        emit activateFunctionInstanceWindow(data);
        win->exec();
    }
}
