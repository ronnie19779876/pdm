#include "baseinputwindow.h"

#include <QTableWidget>
#include <QMenu>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QPushButton>
#include <QHeaderView>
#include <QDir>
#include <QFileDialog>
#include <QApplication>
#include <QClipboard>

BaseInputWindow::BaseInputWindow(QWidget* parent) : QDialog(parent), twConfigure(new QTableWidget(this)), meuPopupMenu(new QMenu(this))
{
    this->tbTenant = new QLineEdit;
    this->tbTenant->setReadOnly(true);
    this->tbNamespace = new QLineEdit;
    this->tbNamespace->setReadOnly(true);
    this->tbName = new QLineEdit;
    this->spParallelism = new QSpinBox;
    this->spParallelism->setRange(1, 3);
    this->spParallelism->setSingleStep(1);
    this->tbFile = new QLineEdit;

    QStringList header;
    header << tr("Key") << tr("Value");
    this->twConfigure->setColumnCount(header.length());
    this->twConfigure->setHorizontalHeaderLabels(header);
    this->twConfigure->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twConfigure->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twConfigure->horizontalHeader()->setStretchLastSection(true);
    this->twConfigure->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twConfigure->verticalHeader()->setHidden(true);
    this->twConfigure->setFocusPolicy(Qt::NoFocus);
    this->twConfigure->setContextMenuPolicy(Qt::CustomContextMenu);
    this->twConfigure->setHidden(true);

#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Save"));
    this->btnCancel = new QPushButton(tr("&Don't Save"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":ok"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    this->btnOk->setDefault(true);
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));
#endif
    this->btnOk->setDefault(true);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowCloseButtonHint);
}

void BaseInputWindow::afterWindowActivated(const QVariant& _var)
{
    this->m_Variant = _var;

    QVBoxLayout* layout = new QVBoxLayout;
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->addRow(tr("&Tenant:"), this->tbTenant);
    formLayout->addRow(tr("N&amespace:"), this->tbNamespace);
    formLayout->addRow(tr("&Name:"), this->tbName);
    formLayout->addRow(tr("&Parallelism:"), this->spParallelism);
    additionalWidget(formLayout);

    QHBoxLayout* fileLayout = new QHBoxLayout;
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->addWidget(this->tbFile);
    QIcon fileIcon = QIcon::fromTheme("folder", QIcon(":/folder"));
    this->btnBrowse = new QPushButton(fileIcon, tr("Browse..."));
    fileLayout->addWidget(this->btnBrowse);
    formLayout->addRow(tr("Archive:"), fileLayout);
    if (needConfigurationTable())
    {
        this->twConfigure->setHidden(false);
        formLayout->addRow(tr("Con&figures:"), this->twConfigure);
    }

    QHBoxLayout* btnLayout = new QHBoxLayout;
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(this->btnOk);
    btnLayout->addSpacing(5);
    btnLayout->addWidget(this->btnCancel);
    layout->addLayout(formLayout);
    layout->addLayout(btnLayout);
    setLayout(layout);

    createActions();

    connect(this->btnCancel, &QPushButton::clicked, this, &BaseInputWindow::close);
    connect(this->btnBrowse, &QPushButton::clicked, this, &BaseInputWindow::handleBrowseFiles);
    connect(this->twConfigure, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleTableContextMenu(QPoint)));
}

void BaseInputWindow::additionalWidget(QFormLayout*) {}

bool BaseInputWindow::needConfigurationTable()
{
    return true;
}

void BaseInputWindow::createActions()
{
    //popup menu actions
    QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/copy-cell"));
    this->actCopyCellText = new QAction(copyIcon, tr("&Copy Cell"));
    this->actAddRow = new QAction(QIcon(":/addnew"), tr("&Add Row"));
    this->actDeleteRow = new QAction(QIcon(":/remove"), tr("&Delete Row"));

    this->meuPopupMenu->addAction(this->actAddRow);
    this->meuPopupMenu->addAction(this->actDeleteRow);
    this->meuPopupMenu->addSeparator();
    this->meuPopupMenu->addAction(this->actCopyCellText);

    connect(this->actAddRow, &QAction::triggered, this, &BaseInputWindow::handleAddRow);
    connect(this->actDeleteRow, &QAction::triggered, this, &BaseInputWindow::handleDeleteRow);
    connect(this->actCopyCellText, &QAction::triggered, this, &BaseInputWindow::handleCopyCellText);
}

void BaseInputWindow::handleBrowseFiles()
{
    QDir current = QDir::currentPath();
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Select NAR or Java Archive File"), current.absoluteFilePath("connectors"), tr("NAR Archive (*.nar);;Java Archive (*.jar)"));
    if (!fileName.isEmpty())
    {
        this->tbFile->setText(fileName);
    }
}

void BaseInputWindow::handleTableContextMenu(const QPoint&)
{
    this->meuPopupMenu->exec(QCursor::pos());
}

void BaseInputWindow::handleAddRow(bool)
{
    this->twConfigure->insertRow(this->twConfigure->rowCount());
    this->twConfigure->setRowCount(this->twConfigure->rowCount());
    if (this->twConfigure->rowCount() > 0)
    {
        this->actDeleteRow->setEnabled(true);
    }
}

void BaseInputWindow::handleDeleteRow(bool)
{
    QTableWidgetItem* item = this->twConfigure->currentItem();
    if (item != Q_NULLPTR)
    {
        int row = item->row();
        if (row >= 0)
        {
            this->twConfigure->removeRow(row);
        }
        if (this->twConfigure->rowCount() <= 0)
        {
            this->actDeleteRow->setEnabled(false);
        }
    }
}

void BaseInputWindow::handleCopyCellText(bool)
{
    QTableWidgetItem* item = this->twConfigure->currentItem();
    if (item && item->isSelected())
    {
        QClipboard* board = QApplication::clipboard();
        board->setText(item->text());
    }
}
