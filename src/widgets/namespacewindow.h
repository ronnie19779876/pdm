#ifndef NAMESPACEWINDOW_H
#define NAMESPACEWINDOW_H

#include <QDialog>

#include "../namespace.h"

class QLineEdit;
class QPushButton;
class QMultiComboBox;
class QComboBox;
class NamespaceService;
class PermissionService;
class TokenService;

class NamespaceWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NamespaceWindow(QWidget* parent = nullptr);

signals:
    void completedNewNamespace(const Namespace& _namespace);

public slots:
    void afterWindowActivated(const Tenant& _tenant);

private:
    QLineEdit* tbNamespace;
    QMultiComboBox* cbActions;
    QComboBox* cbRoles;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    Tenant m_Tenant;

    NamespaceService* m_NamespaceService;
    PermissionService* m_PermissionService;
    TokenService* m_TokenService;

private slots:
    void handleNewNamespace();

};

#endif // NAMESPACEWINDOW_H
