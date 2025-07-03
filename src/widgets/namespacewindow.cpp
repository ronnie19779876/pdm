#include "namespacewindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QComboBox>
#include <QMessageBox>

#include "../qmulticombobox.h"
#include "../services/namespaceservice.h"
#include "../services/permissionservice.h"
#include "../services/tokenservice.h"

NamespaceWindow::NamespaceWindow(QWidget* parent) : QDialog(parent), m_NamespaceService(new NamespaceService(this)), m_PermissionService(new PermissionService(this))
    , m_TokenService(new TokenService(this))
{
    this->m_TokenService->readTokens();
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->tbNamespace = new QLineEdit;
    formLayout->addRow(tr("&Namespaces:"), this->tbNamespace);
    this->cbRoles = new QComboBox;
    this->cbRoles->addItems(this->m_TokenService->tokens());
    this->cbActions = new QMultiComboBox(this->m_PermissionService->actions());
    formLayout->addRow(tr("&Authorization:"), this->cbRoles);
    formLayout->addRow(tr(""), this->cbActions);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Save"));
    this->btnCancel = new QPushButton(tr("&Don't Save"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/images/ok.ico"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    this->btnOk->setDefault(true);
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/images/cancel.ico"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(this->btnOk);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(this->btnCancel);

    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedSize(QSize(500, 160));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Namespace"));
    setWindowIcon(QIcon(":/namespace"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &NamespaceWindow::handleNewNamespace);
    connect(this->btnCancel, &QPushButton::clicked, this, &NamespaceWindow::close);
}

void NamespaceWindow::afterWindowActivated(const Tenant& _tenant)
{
    this->m_Tenant = _tenant;
    if (!this->m_Tenant.authToken().isEmpty())
    {
        m_NamespaceService->setAuthToken(this->m_Tenant.authToken());
    }
}

void NamespaceWindow::handleNewNamespace()
{
    if (this->tbNamespace->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Namespace name is required."));
        this->tbNamespace->setFocus();
        return;
    }
    QStringList selected = this->cbActions->selectedItems();
    if (selected.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Authorization's role is required."));
        this->cbActions->setFocus();
        return;
    }
    Namespace ns;
    ns.setName(this->tbNamespace->text());
    ns.setTenant(this->m_Tenant);
    ns.setRole(this->cbRoles->currentText());
    ns.setAuths(selected);
    HttpStatusCode error;
    this->m_NamespaceService->createNamespace(ns, error);
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        emit completedNewNamespace(ns);
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}
