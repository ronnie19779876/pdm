#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QMdiArea;
class QTreeWidget;
class QToolBar;
class QTreeWidgetItem;
class ClusterService;
class TenantService;
class NamespaceService;
class Cluster;
class Tenant;
class Namespace;

#include "mdisubwindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();

Q_SIGNALS:
    void loadClusters();
    void activateNewClusterWindow(const Cluster* _cluster = nullptr);
    void activateNewTenantWindow(const Cluster& _cluster);
    void activateNewNamespaceWindow(const Tenant& _tenant);
    void activateTopicSubWindow(const QVariant&);
    void activateFunctionSubWindow(const QVariant&);
    void activateSourceSubWindow(const QVariant&);
    void activateSinkSubWindow(const QVariant&);
    void activatePermissionWindow(const Namespace& _namespace);

protected:
    void closeEvent(QCloseEvent* event) override;

private:
    void createActions();
    void createTenantsDock();
    void createClustersDock();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    MdiSubWindow* findMdiSubWindow(const MdiSubWindow::SubWindowType& _type) const;

private slots:
    void handleLoadTenantsAndNamespaces(const Cluster& _cluster);
    void handleExpandAll(bool);
    void handleCollapseAll(bool);
    void handleReflashAll(bool);
    void handleLoadClusters();
    void handleNewClusterWindow(bool);
    void handleUpdateClusterWindow(bool);
    void handleNewTenantWindow(bool);
    void handleNewNamespaceWindow(bool);
    void handleNewTokenWindow(bool);
    void handleFunctionWindow(bool);
    void handleSourceWindow(bool);
    void handleSinkWindow(bool);
    void handlePermissionWindow(bool);
    void handleOpenCluster(bool);
    void handleCloseCluster(bool);
    void handleInsertTreeItem(const Cluster& _cluster);
    void handleInsertTreeItem(const Tenant& _tenant);
    void handleInsertTreeItem(const Namespace& _namespace);
    void handleUpdateTreeItem(const Cluster& _cluster);
    void handleTreeItemDoubleClicked(QTreeWidgetItem* _item, int _column);
    void handleTreeItemPressed(QTreeWidgetItem* _item, int _column);
    void handleTreeCurrentItemChanged(QTreeWidgetItem* _current, QTreeWidgetItem* _previous);
    void handleDeleteCluster(bool);
    void handleDeleteTenant(bool);
    void handleDeleteNamespace(bool);
    void updateMenus();
    void updateWindowMenu();

private:
    QMdiArea* mdiMain;
    QTreeWidget* treeTenants;
    QTreeWidget* treeClusters;
    QMenu* menuView;
    QMenu* menuWindow;
    QToolBar* tlbTenantToolbar;
    QToolBar* tlbClusterToolbar;

    QAction* actTokens;
    QAction* actNewTenant;
    QAction* actNewCluster;
    QAction* actDelCluster;
    QAction* actEditCluster;
    QAction* actOpenCluster;
    QAction* actCloseCluster;
    QAction* actDelTenant;
    QAction* actNewNamespace;
    QAction* actDelNamespace;
    QAction* actFunctions;
    QAction* actSinks;
    QAction* actSources;
    QAction* actPermission;

    QAction* closeAct;
    QAction* closeAllAct;
    QAction* tileAct;
    QAction* cascadeAct;
    QAction* nextAct;
    QAction* previousAct;
    QAction* windowMenuSeparatorAct;

    ClusterService* m_ClusterService;
    TenantService* m_TenantService;
    NamespaceService* m_NamespaceService;
    //const QTreeWidgetItem* m_CurrentItem;

};
#endif // MAINWINDOW_H
