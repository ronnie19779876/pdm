#ifndef TENANTWINDOW_H
#define TENANTWINDOW_H

#include <QDialog>

class QLineEdit;
class QComboBox;
class QPushButton;
class TenantService;
class ClusterService;

#include "../tenant.h"

class TenantWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TenantWindow(QWidget* parent = nullptr);

public slots:
    void afterWindowActivated(const Cluster& _cluster);

signals:
    void completedNewTenant(const Tenant& _tenant);

private:
    QLineEdit* tbTenant;
    QComboBox* cbCluster;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    TenantService* m_TenantService;
    ClusterService* m_ClusterService;

    Cluster m_Cluster;

private slots:
    void handleNewTenant();

};

#endif // TENANTWINDOW_H
