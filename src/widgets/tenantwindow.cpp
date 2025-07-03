#include "tenantwindow.h"

#include <QIcon>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>

#include "../services/tenantservice.h"
#include "../services/clusterservice.h"

TenantWindow::TenantWindow(QWidget* _parent) : QDialog(_parent), m_TenantService(new TenantService(this)), m_ClusterService(new ClusterService(this))
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLayout->setSpacing(10);
    this->tbTenant = new QLineEdit;
    this->cbCluster = new QComboBox;
    formLayout->addRow(tr("&Tenants:"), this->tbTenant);
    formLayout->addRow(tr("Allowed &Clusters:"), this->cbCluster);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Save"));
    this->btnCancel = new QPushButton(tr("&Don't Save"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(this->btnOk);
    buttonLayout->addWidget(this->btnCancel);
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedSize(QSize(420, 130));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Tenant"));
    setWindowIcon(QIcon(":/tenant"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &TenantWindow::handleNewTenant);
    connect(this->btnCancel, &QPushButton::clicked, this, &TenantWindow::close);
}

void TenantWindow::afterWindowActivated(const Cluster& _cluster)
{
    this->m_Cluster = _cluster;
    if (!this->m_Cluster.authtoken().isEmpty())
    {
        m_TenantService->setAuthToken(this->m_Cluster.authtoken());
        m_ClusterService->setAuthToken(this->m_Cluster.authtoken());
    }
    this->cbCluster->addItems(this->m_ClusterService->clusters(this->m_Cluster));
}

void TenantWindow::handleNewTenant()
{
    if (this->tbTenant->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Tenant name is required."));
        this->tbTenant->setFocus();
        return;
    }
    HttpStatusCode error;
    Tenant tenant(this->m_Cluster);
    tenant.setName(this->tbTenant->text());
    tenant.addCluster(this->cbCluster->currentText());
    this->m_TenantService->createTenant(tenant, error);
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        emit completedNewTenant(tenant);
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}
