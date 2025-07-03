#include "basemdisubwindow.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QVBoxLayout>
#include <QToolBar>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

#include "../services/httpclient.h"

BaseMdiSubWindow::BaseMdiSubWindow(QWidget* parent) : MdiSubWindow(parent), twTable(new QTableWidget(this)), meuPopupMenu(new QMenu(this)), tbToolbar(new QToolBar(this)), m_HasCreateActions(false)
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(1);

    //tbToolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    //this->tbToolbar->setIconSize(QSize(16, 16));
    layout->addWidget(tbToolbar);

    this->twTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twTable->horizontalHeader()->setStretchLastSection(true);
    this->twTable->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twTable->verticalHeader()->setHidden(true);
    this->twTable->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twTable->setFocusPolicy(Qt::NoFocus);
    this->twTable->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twTable);

    QWidget* widget = new QWidget(this);
    widget->setLayout(layout);
    setWidget(widget);

    setAttribute(Qt::WA_DeleteOnClose);

    connect(this->twTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleTableContextMenu(QPoint)));
    connect(this, &BaseMdiSubWindow::removeTableItem, this, &BaseMdiSubWindow::handleRemoveTableItem);
}

MdiSubWindow::SubWindowType BaseMdiSubWindow::subWindowType() const
{
    return MdiSubWindow::subWindowType();
}

void BaseMdiSubWindow::afterWindowActivated(const QVariant& _variant)
{
    this->m_Variant = _variant;
}

void BaseMdiSubWindow::createActions()
{
    //toolbar actions
    this->actNew = new QAction(QIcon(":/addnew"), QString(), this);
    this->actStart = new QAction(QIcon(":/start"), QString(), this);
    this->actStop = new QAction(QIcon(":/stop"), QString(), this);
    this->actDelete = new QAction(QIcon(":/remove"), QString(), this);
    this->actRefresh = new QAction(QIcon(":/refresh"), QString(), this);
    this->actClose = new QAction(QIcon(":/exit"), tr("&Close"), this);
    this->actClose->setStatusTip(tr("Close the window."));

    connect(this->actClose, &QAction::triggered, this, &BaseMdiSubWindow::close);
    connect(this->actDelete, &QAction::triggered, this, &BaseMdiSubWindow::handleDelete);
    connect(this->actStart, &QAction::triggered, this, &BaseMdiSubWindow::handleStart);
    connect(this->actStop, &QAction::triggered, this, &BaseMdiSubWindow::handleStop);

    //popup menu actions
    this->actUpdate = new QAction(QIcon(":/update"), QString(), this);
    QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/copy-cell"));
    this->actCopyCellText = new QAction(copyIcon, tr("&Copy Cell"));
    this->actCopyRowText = new QAction(QIcon(":/copy-row"), tr("&Copy Row"));
    this->actInstances =  new QAction(QIcon(":/overview"), QString(), this);
    this->meuPopupMenu->addAction(this->actUpdate);
    this->meuPopupMenu->addAction(this->actInstances);
    this->meuPopupMenu->addSeparator();
    this->meuPopupMenu->addAction(this->actCopyCellText);
    this->meuPopupMenu->addAction(this->actCopyRowText);

    connect(this->actCopyCellText, &QAction::triggered, this, &BaseMdiSubWindow::handleCopyCellText);
    connect(this->actCopyRowText, &QAction::triggered, this, &BaseMdiSubWindow::handleCopyRowText);

    //Log4Qt::Logger::logger("main")->debug("BaseMdiSubWindow::createActions()");
}

void BaseMdiSubWindow::createToolbar()
{
    this->tbToolbar->addAction(this->actNew);
    this->tbToolbar->addAction(this->actDelete);
    this->tbToolbar->addAction(this->actRefresh);
    this->tbToolbar->addSeparator();
    this->tbToolbar->addAction(this->actStart);
    this->tbToolbar->addAction(this->actStop);
    this->tbToolbar->addSeparator();
    this->tbToolbar->addAction(this->actClose);
}

bool BaseMdiSubWindow::doDelete(const QVariant&, HttpStatusCode&) { return false; }

bool BaseMdiSubWindow::doStart(const QVariant&, HttpStatusCode&) { return false; }

bool BaseMdiSubWindow::doStop(const QVariant&, HttpStatusCode&) { return false; }

void BaseMdiSubWindow::handleTableContextMenu(const QPoint& _pos)
{
    Q_UNUSED(_pos);
    this->meuPopupMenu->exec(QCursor::pos());
}

void BaseMdiSubWindow::handleCopyCellText(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QClipboard* board = QApplication::clipboard();
        board->setText(item->text());
    }
}

void BaseMdiSubWindow::handleCopyRowText(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QString text;
        int row = item->row();
        int columns = this->twTable->columnCount();
        for (int i = 0, n = columns; i < n; ++i)
        {
            QTableWidgetItem* cell = this->twTable->item(row, i);
            text.append(cell->text()).append('\t');
        }
        QClipboard* board = QApplication::clipboard();
        board->setText(text);
    }
}

void BaseMdiSubWindow::handleRemoveTableItem(const QTableWidgetItem& _item)
{
    int row = _item.row();
    if (row >= 0)
    {
        this->twTable->removeRow(row);
    }
}

void BaseMdiSubWindow::handleDelete(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Function"), tr("Are you sure you want to delete this function?"));
        if (button == QMessageBox::Yes)
        {
            item = this->twTable->item(item->row(), 0);
            QVariant data = item->data(Qt::UserRole);
            HttpStatusCode error;
            bool ok = this->doDelete(data, error);
            if (ok)
            {
                emit removeTableItem(*item);
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void BaseMdiSubWindow::handleStart(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Start Function"), tr("Are you sure you want to start this function?"));
        if (button == QMessageBox::Yes)
        {
            this->actStart->setEnabled(false);
            item = this->twTable->item(item->row(), 0);
            QVariant data = item->data(Qt::UserRole);
            HttpStatusCode error;
            bool ok = this->doStart(data, error);
            if (ok)
            {
                emit completeStart();
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
            this->actStart->setEnabled(true);
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void BaseMdiSubWindow::handleStop(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Stop Function"), tr("Are you sure you want to stop this function?"));
        if (button == QMessageBox::Yes)
        {
            this->actStop->setEnabled(false);
            item = this->twTable->item(item->row(), 0);
            QVariant data = item->data(Qt::UserRole);
            HttpStatusCode error;
            bool ok = this->doStop(data, error);
            if (ok)
            {
                emit completeStop();
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
            this->actStop->setEnabled(true);
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}
