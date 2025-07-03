#ifndef PERMISSIONSWINDOW_H
#define PERMISSIONSWINDOW_H

#include <QDialog>

#include "../namespace.h"

class QTreeWidget;
class PermissionService;
class TokenService;
class Role;
class QTreeWidgetItem;
class QLabel;
class QComboBox;
class QMultiComboBox;

class PermissionsWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PermissionsWindow(QWidget* parent = nullptr);

signals:
    void completeWindowActivated();
    void completeRevokePermission(QTreeWidgetItem*);
    void completeGrantPermission(const Role&);

public slots:
    void afterWindowActivated(const Namespace&);

private:
    PermissionService* m_PermissionService;
    TokenService* m_TokenService;
    Namespace m_Namespace;

    QLabel* lblTenant;
    QLabel* lblNamespace;
    QComboBox* cobRoles;
    QMultiComboBox* mcobPermissions;
    QTreeWidget* treeRoles;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QAction* actDeleteRole;

private slots:
    void initialize();
    void handleNewRole(bool);
    void handleDeleteRole(bool);
    void handleInsertNewRole(const Role&);
    void handleRemoveTreeItem(QTreeWidgetItem*);
    void handleTreeItemPressed(QTreeWidgetItem*, int);

};

#endif // PERMISSIONSWINDOW_H
