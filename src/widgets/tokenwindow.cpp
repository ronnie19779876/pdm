#include "tokenwindow.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QFileDialog>
#include <QTableWidget>
#include <QHeaderView>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

#include "../services/tokenservice.h"

TokenWindow::TokenWindow(QWidget* parent) : QDialog(parent), twTable(new QTableWidget(this)), meuPopupMenu(new QMenu(this)), m_TokenService(new TokenService(this))
{
    m_TokenService->readTokens();
    QVBoxLayout* layout = new QVBoxLayout;

    //表单布局
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->tbSubject = new QLineEdit;
    this->tbKey =  new QLineEdit;

    QHBoxLayout* fileLayout = new QHBoxLayout;
    fileLayout->setContentsMargins(0, 0, 0, 0);
    fileLayout->addWidget(this->tbKey);
    QIcon fileIcon = QIcon::fromTheme("folder", QIcon(":/folder"));
    this->btnSelectFile = new QPushButton(fileIcon, tr("Browse..."));
    fileLayout->addWidget(this->btnSelectFile);

    formLayout->addRow(tr("Token &Subject:"), this->tbSubject);
    formLayout->addRow(tr("SecretKey &File:"), fileLayout);

    this->twTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twTable->horizontalHeader()->setStretchLastSection(true);
    this->twTable->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twTable->verticalHeader()->setHidden(true);
    this->twTable->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twTable->setFocusPolicy(Qt::NoFocus);
    this->twTable->setContextMenuPolicy(Qt::CustomContextMenu);

    QHBoxLayout* btnLayout = new QHBoxLayout;
    this->btnGenerateKey = new QPushButton(tr("&Generate Key..."));
#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Yes"));
    this->btnCancel = new QPushButton(tr("&No"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    this->btnOk->setDefault(true);
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));
#endif
    this->btnOk->setDefault(true);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(this->btnGenerateKey);
    btnLayout->addWidget(this->btnOk);
    btnLayout->addSpacing(5);
    btnLayout->addWidget(this->btnCancel);

    layout->addLayout(formLayout);
    layout->addWidget(this->twTable);
    layout->addLayout(btnLayout);

    setLayout(layout);
    setFixedHeight(350);
    setFixedWidth(500);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Token"));
    setWindowIcon(QIcon(":/token"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &TokenWindow::handleNewToken);
    connect(this->btnCancel, &QPushButton::clicked, this, &TokenWindow::close);
    connect(this->btnSelectFile, &QPushButton::clicked, this, &TokenWindow::handleSelectedFile);
    connect(this->btnGenerateKey, &QPushButton::clicked, this, &TokenWindow::handleGenerateSecretKey);
    connect(this->twTable, &QTableWidget::customContextMenuRequested, this, &TokenWindow::handleTableContextMenu);
    connect(this, &TokenWindow::completedAddnewToken, this, &TokenWindow::handleInsertTableItem);
    connect(this, &TokenWindow::removeTableItem, this, &TokenWindow::handleRemoveTableItem);

    QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/copy-cell"));
    this->actCopyCellText = new QAction(copyIcon, tr("&Copy Cell"));
    this->actDelete = new QAction(QIcon(":/remove"), tr("&Remove Token"));
    this->actDelete->setStatusTip(tr("Delete a token."));
    this->meuPopupMenu->addAction(this->actDelete);
    this->meuPopupMenu->addSeparator();
    this->meuPopupMenu->addAction(this->actCopyCellText);
    connect(this->actDelete, &QAction::triggered, this, &TokenWindow::handleDeleteToken);
    connect(this->actCopyCellText, &QAction::triggered, this, &TokenWindow::handleCopyCellText);

    initialize();
}

TokenWindow::~TokenWindow()
{
    this->m_TokenService->writeTokens();
}

void TokenWindow::initialize()
{
    this->m_Headers << tr("Name") << tr("Token");
    this->twTable->setColumnCount(this->m_Headers.length());
    this->twTable->setHorizontalHeaderLabels(this->m_Headers);

    this->twTable->clearContents();
    QList<Token> tokens = this->m_TokenService->getTokens();
    this->twTable->setRowCount(tokens.length());
    for (int i = 0, n = tokens.length(); i < n; i++)
    {
        handleInsertTableItem(i, tokens[i]);
    }
    this->twTable->repaint();
}

void TokenWindow::handleNewToken(bool)
{
    if (this->tbSubject->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Token subject is required."));
        this->tbSubject->setFocus();
        return;
    }

    if (this->tbKey->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("SecretKey file is required."));
        this->tbKey->setFocus();
        return;
    }

    QFile file(this->tbKey->text());
    if (!file.exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("SecretKey file not exist."));
        this->tbKey->setFocus();
        return;
    }
    file.open(QFile::ReadOnly);
    QByteArray key = file.readAll();
    if (key.length() != 32)
    {
        QMessageBox::critical(this, tr("Error"), tr("invalid format SecretKey file."));
        this->tbKey->setFocus();
        return;
    }

    if (this->m_TokenService->exist(this->tbSubject->text()))
    {
        QMessageBox::critical(this, tr("Error"), tr("Token subject already exists."));
        this->tbSubject->setFocus();
    }
    else
    {
        Token token = this->m_TokenService->fromJsonWebToken(key, this->tbSubject->text());
        if (!token.authtoken().isEmpty())
        {
            this->m_TokenService->addToken(token);
            this->tbSubject->clear();
            this->tbKey->clear();
            QMessageBox::information(this, tr("Success"), tr("Generate token successfully."));
            int row = this->twTable->rowCount();
            this->twTable->setRowCount(row + 1);
            emit completedAddnewToken(row, token);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Generate token fail."));
        }
    }
    file.close();
}

void TokenWindow::handleSelectedFile(bool)
{
    const QString fileName = QFileDialog::getOpenFileName(this, tr("Select SecretKey File"), QDir::currentPath(), tr("Key File(*.key)"));
    if (!fileName.isEmpty())
    {
        this->tbKey->setText(fileName);
    }
}

void TokenWindow::handleGenerateSecretKey(bool)
{
    const QString fileName = QFileDialog::getSaveFileName(this, tr("Save SecretKey File"), QDir::currentPath(), tr("Key File(*.key)"));
    if (!fileName.isEmpty())
    {
        if (this->m_TokenService->generateSecretKey(fileName))
        {
            QMessageBox::information(this, tr("Success"), tr("Generate SecretKey file successfully."));
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("invalid format SecretKey file."));
        }
    }
}

void TokenWindow::handleTableContextMenu(const QPoint&)
{
    this->meuPopupMenu->exec(QCursor::pos());
}

void TokenWindow::handleCopyCellText(bool)
{
    QTableWidgetItem* item = this->twTable->currentItem();
    if (item && item->isSelected())
    {
        QClipboard* board = QApplication::clipboard();
        board->setText(item->text());
    }
}

void TokenWindow::handleInsertTableItem(const int& row, const Token& _token)
{
    QStringList columns;
    columns << "name" << "authToken";
    DataModel data = _token.toData();
    for (int i = 0, n = columns.length(); i < n; i++)
    {
        QTableWidgetItem* item = new QTableWidgetItem(data[columns[i]]);
        item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
        this->twTable->setItem(row, i, item);
        if (i == 0)
            item->setData(Qt::UserRole, QVariant::fromValue(_token));
    }
}

void TokenWindow::handleDeleteToken(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Token"), tr("Are you sure you want to delete this token?"));
        if (button == QMessageBox::Yes)
        {
            int row = current->row();
            QTableWidgetItem* item = this->twTable->item(row, 0);
            QVariant data = item->data(Qt::UserRole);
            if (data.canConvert<Token>())
            {
                Token token = data.value<Token>();
                this->m_TokenService->removeToken(token);
                emit removeTableItem(*item);
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TokenWindow::handleRemoveTableItem(const QTableWidgetItem& _item)
{
    int row = _item.row();
    if (row >= 0)
    {
        this->twTable->removeRow(row);
    }
}
