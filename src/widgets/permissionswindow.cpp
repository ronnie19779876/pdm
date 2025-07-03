#include "permissionswindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QLabel>
#include <QComboBox>
#include <QTreeWidget>
#include <QPushButton>
#include <QApplication>
#include <QMenu>

#include "../services/permissionservice.h"
#include "../services/tokenservice.h"
#include "../qmulticombobox.h"

PermissionsWindow::PermissionsWindow(QWidget* parent) : QDialog(parent), m_PermissionService(new PermissionService(this)), m_TokenService(new TokenService(this))
{
    //创建水平布局
    QHBoxLayout* layout = new QHBoxLayout;

    //实例化组件
    this->lblTenant = new QLabel;
    this->lblNamespace = new QLabel;
    this->cobRoles = new QComboBox;
    this->mcobPermissions = new QMultiComboBox(this->m_PermissionService->actions());
    this->treeRoles = new QTreeWidget(this);
    this->treeRoles->setHeaderHidden(true);
    this->btnOk = new QPushButton(tr("&Grant"));
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));

    QFormLayout* formLeftLayout = new QFormLayout;
    formLeftLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    formLeftLayout->addRow(tr("&Tenant:"), this->lblTenant);
    formLeftLayout->addRow(tr("N&amespace:"), this->lblNamespace);
    formLeftLayout->addRow(tr("&Authorization:"), this->cobRoles);
    formLeftLayout->addRow(tr(""), this->mcobPermissions);
    //formLeftLayout->addRow(new QLabel("\n\n"));
    formLeftLayout->addRow(this->btnOk);
    formLeftLayout->addRow(this->btnCancel);

    QFormLayout* formRightLayout = new QFormLayout;
    formRightLayout->addRow(tr("&Permissions:"), new QLabel(""));
    formRightLayout->addRow(this->treeRoles);

    layout->addLayout(formLeftLayout);
    layout->addLayout(formRightLayout);

    setLayout(layout);
    setFixedWidth(800);
    setFixedHeight(220);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Permission Settings"));
    setWindowIcon(QIcon(":/permission"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    this->actDeleteRole = new QAction(tr("&Revoke"), this->treeRoles);
    this->actDeleteRole->setStatusTip(tr("Revoke all permissions to a role on a namespace."));

    connect(this->actDeleteRole, &QAction::triggered, this, &PermissionsWindow::handleDeleteRole);
    connect(this, &PermissionsWindow::completeWindowActivated, this, &PermissionsWindow::initialize);
    connect(this->btnOk, &QPushButton::clicked, this, &PermissionsWindow::handleNewRole);
    connect(this->btnCancel, &QPushButton::clicked, this, &PermissionsWindow::close);
    connect(this->treeRoles, &QTreeWidget::itemPressed, this, &PermissionsWindow::handleTreeItemPressed);
    connect(this, &PermissionsWindow::completeRevokePermission, this, &PermissionsWindow::handleRemoveTreeItem);
    connect(this, &PermissionsWindow::completeGrantPermission, this, &PermissionsWindow::handleInsertNewRole);
}

void PermissionsWindow::afterWindowActivated(const Namespace& _namespace)
{
    this->m_Namespace = _namespace;
    if (!this->m_Namespace.authToken().isEmpty())
    {
        this->m_PermissionService->setAuthToken(this->m_Namespace.authToken());
    }
    this->m_TokenService->readTokens();
    emit completeWindowActivated();
}

void PermissionsWindow::initialize()
{
    this->lblTenant->setText(this->m_Namespace.tenant().name());
    this->lblNamespace->setText(this->m_Namespace.name());
    this->cobRoles->addItems(this->m_TokenService->tokens());
    HttpStatusCode error;
    QList<Role> roles = this->m_PermissionService->roles(this->m_Namespace, error);
    if (error.code == HttpStatusCode::StatusCode::OK)
    {
        Q_FOREACH (const Role& role, roles)
        {
            QTreeWidgetItem* item = new  QTreeWidgetItem(this->treeRoles);
            item->setText(0, role.name());
            item->setIcon(0, QIcon(":/role"));
            item->setData(0, Qt::UserRole, QVariant::fromValue(role));

            QStringList actions = role.permissions();
            foreach (QString action, actions)
            {
                QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
                subitem->setText(0, action);
                subitem->setIcon(0, QIcon(":/token"));
            }
            this->treeRoles->insertTopLevelItem(0, item);
        }
        this->treeRoles->expandAll();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}

void PermissionsWindow::handleNewRole(bool checked)
{
    Q_UNUSED(checked);
    QStringList actions = this->mcobPermissions->selectedItems();
    if (actions.isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Authorization's role is required."));
        this->mcobPermissions->setFocus();
    }
    else
    {
        Role role;
        role.setNamespace(this->m_Namespace);
        role.setName(this->cobRoles->currentText());
        role.setPermissions(actions);
        HttpStatusCode error;
        this->m_PermissionService->grant(role, error);
        if (error.code == HttpStatusCode::StatusCode::NoContent)
        {
            this->mcobPermissions->reset();
            emit completeGrantPermission(role);
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), error.errorString());
        }
    }
}

void PermissionsWindow::handleDeleteRole(bool checked)
{
    Q_UNUSED(checked);
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Revoke Permission"), tr("Are you sure you want to revoke this permission?"));
    if (button == QMessageBox::Yes)
    {
        QTreeWidgetItem* item = this->treeRoles->currentItem();
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Role>())
        {
            HttpStatusCode error;
            this->m_PermissionService->revoke(data.value<Role>(), error);
            if (error.code == HttpStatusCode::StatusCode::NoContent)
            {
                emit completeRevokePermission(item);
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
    }
}

void PermissionsWindow::handleInsertNewRole(const Role& _role)
{
    Q_UNUSED(_role);
    QTreeWidgetItem* item = new  QTreeWidgetItem(this->treeRoles);
    item->setText(0, _role.name());
    item->setIcon(0, QIcon(":/role"));
    item->setData(0, Qt::UserRole, QVariant::fromValue(_role));
    QStringList actions = _role.permissions();
    Q_FOREACH (const QString& action, actions)
    {
        QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
        subitem->setText(0, action);
        subitem->setIcon(0, QIcon(":/token"));
    }
    this->treeRoles->insertTopLevelItem(0, item);
    this->treeRoles->expandAll();
}

void PermissionsWindow::handleRemoveTreeItem(QTreeWidgetItem* _item)
{
    delete _item;
}

void PermissionsWindow::handleTreeItemPressed(QTreeWidgetItem* _item, int _column)
{
    if (qApp->mouseButtons() == Qt::RightButton)
    {
        QMenu* popup = new QMenu(this->treeRoles);
        QVariant data = _item->data(_column, Qt::UserRole);
        if (data.canConvert<Role>())
        {
            popup->addAction(this->actDeleteRole);
        }
        popup->exec(QCursor::pos());
    }
}
