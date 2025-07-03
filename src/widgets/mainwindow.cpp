#include "mainwindow.h"

#include <QApplication>
#include <QDockWidget>
#include <QToolBar>
#include <QMdiArea>
#include <QVBoxLayout>
#include <QTreeWidget>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QCloseEvent>
#include <QSettings>
#include <QScreen>
#include <QMessageBox>
#include <QDebug>
#include <QDir>

#include "../constants.h"
#include "../services/clusterservice.h"
#include "../services/tenantservice.h"
#include "../services/namespaceservice.h"
#include "../widgets/newclusterwindow.h"
#include "../widgets/tenantwindow.h"
#include "../widgets/namespacewindow.h"
#include "../widgets/topicswindow.h"
#include "../widgets/functionswindow.h"
#include "../widgets/sourceswindow.h"
#include "../widgets/sinkswindow.h"
#include "../widgets/permissionswindow.h"
#include "../widgets/tokenwindow.h"

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent),
    mdiMain(new QMdiArea(this)),
    treeTenants(new QTreeWidget(this)),
    treeClusters(new QTreeWidget(this)),
    tlbTenantToolbar(new QToolBar(this)),
    tlbClusterToolbar(new QToolBar(this)),
    m_ClusterService(new ClusterService(this)),
    m_TenantService(new TenantService(this)),
    m_NamespaceService(new NamespaceService(this))
{
    //QDir basePath(QCoreApplication::applicationDirPath());
    //QMessageBox::critical(this, tr("Error"), basePath.absolutePath());
    readSettings();
    createActions();
    createStatusBar();
    createTenantsDock();
    createClustersDock();

    this->mdiMain->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    this->mdiMain->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    //mdiMain->setViewMode(QMdiArea::ViewMode::TabbedView);
    this->mdiMain->setOption(QMdiArea::DontMaximizeSubWindowOnActivation, false);
    setCentralWidget(this->mdiMain);

    updateMenus();

    QString title(APPLICATION_NAME);
    setWindowTitle(title.append(" @ ").append(APPLICATION_VERSION));
    setWindowIcon(QIcon(":/pulsar"));
    setUnifiedTitleAndToolBarOnMac(true);

    connect(this, & MainWindow::loadClusters, this, & MainWindow::handleLoadClusters);
    connect(this->treeTenants, & QTreeWidget::itemPressed, this, & MainWindow::handleTreeItemPressed);
    connect(this->treeTenants, & QTreeWidget::itemDoubleClicked, this, & MainWindow::handleTreeItemDoubleClicked);
    connect(this->mdiMain, & QMdiArea::subWindowActivated, this, & MainWindow::updateMenus);
}

MainWindow::~MainWindow()
{
    qDebug() << "~MainWindow";
}

void MainWindow::closeEvent(QCloseEvent* _event)
{
    mdiMain->closeAllSubWindows();
    if (mdiMain->currentSubWindow())
    {
        _event->ignore();
    }
    else
    {
        writeSettings();
        _event->accept();
    }
}

void MainWindow::createActions()
{
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    this->actTokens = new QAction(QIcon(":/token"), tr("&New Token..."), this);
    this->actTokens->setShortcuts(QKeySequence::New);
    this->actTokens->setStatusTip(tr("Create a new token that allowed you to connect broker."));
    connect(this->actTokens, & QAction::triggered, this, & MainWindow::handleNewTokenWindow);
    fileMenu->addAction(this->actTokens);
    fileMenu->addSeparator();
    const QIcon exitIcon = QIcon::fromTheme("application-exit", QIcon(":/exit"));
    QAction* exitAct = fileMenu->addAction(exitIcon, tr("E&xit"), qApp, & QApplication::closeAllWindows);
    exitAct->setShortcuts(QKeySequence::Quit);
    exitAct->setStatusTip(tr("Exit the application"));
    fileMenu->addAction(exitAct);

    this->menuView = menuBar()->addMenu(tr("&View"));
    this->menuWindow = menuBar()->addMenu(tr("&Window"));
    connect(this->menuWindow, & QMenu::aboutToShow, this, & MainWindow::updateWindowMenu);

    this->actDelTenant = new QAction(QIcon(":/delete-tenant"), tr("&Delete Tenant"), this);
    this->actDelTenant->setStatusTip(tr("Delete a tenant and all namespaces and topics under it."));
    this->actNewNamespace = new QAction(QIcon(":/new-namespace"), tr("&New Namespace..."), this);
    this->actNewNamespace->setStatusTip(tr("Creates a new namespace with the specified policies."));
    this->actDelNamespace = new QAction(QIcon(":/remove"), tr("&Delete Namespace"), this);
    this->actDelNamespace->setStatusTip(tr("Delete a namespace and all the topics under it."));
    this->actFunctions = new QAction(QIcon(":/function"), tr("&Functions..."), this);
    this->actFunctions->setStatusTip(tr("Manage Functions."));
    this->actFunctions->setEnabled(false);
    this->actSinks = new QAction(QIcon(":/export"), tr("S&inks..."), this);
    this->actSinks->setStatusTip(tr("Manage Sinks."));
    this->actSinks->setEnabled(false);
    this->actSources = new QAction(QIcon(":/import"), tr("S&ources..."), this);
    this->actSources->setStatusTip(tr("Manage Sources."));
    this->actSources->setEnabled(false);
    this->actPermission = new QAction(QIcon(":/permission"), tr("&Permission..."), this);
    this->actPermission->setStatusTip(tr("Grant or Revoke a new permission to a role on a single topic."));
    connect(this->actDelTenant, & QAction::triggered, this, & MainWindow::handleDeleteTenant);
    connect(this->actDelNamespace, & QAction::triggered, this, & MainWindow::handleDeleteNamespace);
    connect(this->actNewNamespace, & QAction::triggered, this, & MainWindow::handleNewNamespaceWindow);
    connect(this->actFunctions, & QAction::triggered, this, & MainWindow::handleFunctionWindow);
    connect(this->actSinks, & QAction::triggered, this, & MainWindow::handleSinkWindow);
    connect(this->actSources, & QAction::triggered, this, & MainWindow::handleSourceWindow);
    connect(this->actPermission, & QAction::triggered, this, & MainWindow::handlePermissionWindow);

    closeAct = new QAction(tr("Cl&ose"), this);
    closeAct->setStatusTip(tr("Close the active window"));
    connect(closeAct, & QAction::triggered, this->mdiMain, & QMdiArea::closeActiveSubWindow);

    closeAllAct = new QAction(tr("Close &All"), this);
    closeAllAct->setStatusTip(tr("Close all the windows"));
    connect(closeAllAct, & QAction::triggered, this->mdiMain, & QMdiArea::closeAllSubWindows);

    tileAct = new QAction(tr("&Tile"), this);
    tileAct->setStatusTip(tr("Tile the windows"));
    connect(tileAct, & QAction::triggered, this->mdiMain, & QMdiArea::tileSubWindows);

    cascadeAct = new QAction(tr("&Cascade"), this);
    cascadeAct->setStatusTip(tr("Cascade the windows"));
    connect(cascadeAct, & QAction::triggered, this->mdiMain, & QMdiArea::cascadeSubWindows);

    nextAct = new QAction(tr("Ne&xt"), this);
    nextAct->setShortcuts(QKeySequence::NextChild);
    nextAct->setStatusTip(tr("Move the focus to the next window"));
    connect(nextAct, & QAction::triggered, this->mdiMain, & QMdiArea::activateNextSubWindow);

    previousAct = new QAction(tr("Pre&vious"), this);
    previousAct->setShortcuts(QKeySequence::PreviousChild);
    previousAct->setStatusTip(tr("Move the focus to the previous window"));
    connect(previousAct, & QAction::triggered, this->mdiMain, & QMdiArea::activatePreviousSubWindow);

    windowMenuSeparatorAct = new QAction(this);
    windowMenuSeparatorAct->setSeparator(true);

    updateWindowMenu();
}

void MainWindow::createTenantsDock()
{
    QDockWidget* dock = new QDockWidget(tr("Tenants/Namespaces"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(1);

    this->tlbTenantToolbar->setEnabled(false);
    this->tlbTenantToolbar->setToolButtonStyle(Qt::ToolButtonStyle::ToolButtonFollowStyle);
    //this->tlbTenantToolbar->setIconSize(QSize(16, 16));
    this->actNewTenant = new QAction(QIcon(":/newtenant"), tr("&New Tenant"), this);
    this->actNewTenant->setStatusTip(tr("Create a new Tenant."));
    QAction* actExpandAll = new QAction(QIcon(":/expandall"), tr("&Expand All"), this);
    actExpandAll->setStatusTip(tr("Expand all Tenants and Namespaces."));
    QAction* actCollapseAll = new QAction(QIcon(":/collapseall"), tr("&Collapse All"), this);
    actCollapseAll->setStatusTip(tr("Collapse all Tenants and Namespaces."));
    QAction* actRefresh = new QAction(QIcon(":/refresh"), tr("&Reflesh"), this);
    actRefresh->setStatusTip(tr("Reflesh Tenants and Namespaces."));
    connect(this->actNewTenant, & QAction::triggered, this, & MainWindow::handleNewTenantWindow);
    connect(actExpandAll, & QAction::triggered, this, & MainWindow::handleExpandAll);
    connect(actCollapseAll, & QAction::triggered, this, & MainWindow::handleCollapseAll);
    connect(actRefresh, & QAction::triggered, this, & MainWindow::handleReflashAll);
    this->tlbTenantToolbar->addAction(this->actNewTenant);
    this->tlbTenantToolbar->addSeparator();
    this->tlbTenantToolbar->addAction(actExpandAll);
    this->tlbTenantToolbar->addAction(actCollapseAll);
    this->tlbTenantToolbar->addSeparator();
    this->tlbTenantToolbar->addAction(actRefresh);
    layout->addWidget(this->tlbTenantToolbar);

    this->treeTenants->setHeaderHidden(true);
    layout->addWidget(this->treeTenants);

    QWidget* dockWidget = new QWidget(dock);
    dockWidget->setLayout(layout);
    dock->setWidget(dockWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    this->menuView->addAction(dock->toggleViewAction());
}

void MainWindow::createClustersDock()
{
    QDockWidget* dock = new QDockWidget(tr("Clusters"), this);
    dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(1);

    this->tlbClusterToolbar->setToolButtonStyle(Qt::ToolButtonFollowStyle);
    //this->tlbClusterToolbar->setIconSize(QSize(16, 16));
    this->actNewCluster = new QAction(QIcon(":/new-cluster"), tr("&New Cluster"), this);
    this->actNewCluster->setStatusTip(tr("Create a new cluster."));
    this->actEditCluster = new QAction(QIcon(":/modify"), tr("&Modify Cluster..."), this);
    this->actEditCluster->setStatusTip(tr("Modify an existing cluster."));
    this->actOpenCluster = new QAction(QIcon(":/link"), tr("&Open Cluster"), this);
    this->actOpenCluster->setStatusTip(tr("Open an existing cluster."));
    this->actCloseCluster = new QAction(QIcon(":/broken-link"), tr("&Close Cluster"), this);
    this->actCloseCluster->setStatusTip(tr("Close an existing cluster."));
    this->actDelCluster = new QAction(QIcon(":/remove"), tr("&Delete Cluster"), this);
    this->actDelCluster->setStatusTip(tr("Delete an existing cluster."));
    connect(this->actNewCluster, & QAction::triggered, this, & MainWindow::handleNewClusterWindow);
    connect(this->actEditCluster, & QAction::triggered, this, & MainWindow::handleUpdateClusterWindow);
    connect(this->actDelCluster, & QAction::triggered, this, & MainWindow::handleDeleteCluster);
    connect(this->actOpenCluster, & QAction::triggered, this, & MainWindow::handleOpenCluster);
    connect(this->actCloseCluster, & QAction::triggered, this, & MainWindow::handleCloseCluster);
    this->tlbClusterToolbar->addAction(this->actNewCluster);
    this->tlbClusterToolbar->addAction(this->actEditCluster);
    this->tlbClusterToolbar->addSeparator();
    this->tlbClusterToolbar->addAction(this->actOpenCluster);
    this->tlbClusterToolbar->addAction(this->actCloseCluster);
    this->tlbClusterToolbar->addSeparator();
    this->tlbClusterToolbar->addAction(this->actDelCluster);
    layout->addWidget(this->tlbClusterToolbar);

    this->treeClusters->setHeaderHidden(true);
    connect(this->treeClusters, & QTreeWidget::itemDoubleClicked, this, & MainWindow::handleTreeItemDoubleClicked);
    connect(this->treeClusters, & QTreeWidget::itemPressed, this, & MainWindow::handleTreeItemPressed);
    connect(this->treeClusters, & QTreeWidget::currentItemChanged, this, & MainWindow::handleTreeCurrentItemChanged);
    layout->addWidget(this->treeClusters);

    QWidget* dockWidget = new QWidget(dock);
    dockWidget->setLayout(layout);
    dock->setWidget(dockWidget);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
    this->menuView->addAction(dock->toggleViewAction());
}

void MainWindow::createStatusBar()
{
    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::readSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    const QByteArray geometry = settings.value("geometry", QByteArray()).toByteArray();
    if (geometry.isEmpty())
    {
        const QRect availableGeometry = screen()->availableGeometry();
        resize(availableGeometry.width() / 3, availableGeometry.height() / 2);
        move((availableGeometry.width() - width()) / 2, (availableGeometry.height() - height()) / 2);
    }
    else
    {
        restoreGeometry(geometry);
    }
    this->m_ClusterService->read();
}

void MainWindow::writeSettings()
{
    QSettings settings(QCoreApplication::organizationName(), QCoreApplication::applicationName());
    settings.setValue("geometry", saveGeometry());
    this->m_ClusterService->write();
}

MdiSubWindow* MainWindow::findMdiSubWindow(const MdiSubWindow::SubWindowType& _type) const
{
    Q_FOREACH (QMdiSubWindow* window, this->mdiMain->subWindowList())
    {
        MdiSubWindow* subWindow = qobject_cast<MdiSubWindow*>(window);
        if (subWindow->subWindowType() == _type)
        {
            return subWindow;
        }
    }
    return 0;
}

void MainWindow::handleLoadTenantsAndNamespaces(const Cluster& _cluster)
{
    if (!_cluster.authtoken().isEmpty())
    {
        this->m_TenantService->setAuthToken(_cluster.authtoken());
        this->m_NamespaceService->setAuthToken(_cluster.authtoken());
    }
    this->treeTenants->clear();
    //load tenants and namespaces.
    QList<Tenant> roots = this->m_TenantService->tenants(_cluster);
    Q_FOREACH (const Tenant& tenant, roots)
    {
        QTreeWidgetItem* item = new  QTreeWidgetItem(this->treeTenants);
        item->setText(0, tenant.name());
        item->setIcon(0, QIcon(":/tenant"));
        item->setData(0, Qt::UserRole, QVariant::fromValue(tenant));

        QList<Namespace> namespaces = this->m_NamespaceService->namespaces(tenant);
        foreach (const Namespace& ns, namespaces)
        {
            QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
            subitem->setText(0, ns.name());
            subitem->setIcon(0, QIcon(":/namespace"));
            subitem->setData(0, Qt::UserRole, QVariant::fromValue(ns));
        }
        this->treeTenants->insertTopLevelItem(0, item);
    }
}

void MainWindow::handleExpandAll(bool)
{
    this->treeTenants->expandAll();
}

void MainWindow::handleCollapseAll(bool)
{
    this->treeTenants->collapseAll();
}

void MainWindow::handleReflashAll(bool)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            handleLoadTenantsAndNamespaces(cluster);
        }
    }
}

void MainWindow::handleLoadClusters()
{
    this->treeClusters->clear();
    //load all Clusters into the tree widget.
    QList<Cluster> clusters = this->m_ClusterService->tolist();
    Q_FOREACH (const Cluster& _cluster, clusters)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->treeClusters);
        item->setText(0, _cluster.name());
        item->setIcon(0, QIcon(":/disconnected"));
        item->setData(0, Qt::UserRole, QVariant::fromValue(_cluster));
        this->treeClusters->addTopLevelItem(item);
    }
}

void MainWindow::handleNewClusterWindow(bool)
{
    NewClusterWindow* win = new NewClusterWindow;
    connect(win, SIGNAL(completeNewCluster(Cluster)), this, SLOT(handleInsertTreeItem(Cluster)));
    connect(this, & MainWindow::activateNewClusterWindow, win, & NewClusterWindow::afterWindowActivated);
    emit activateNewClusterWindow();
    win->exec();
}

/**
 * display Cluster window to modify.
 * @brief MainWindow::handleUpdateClusterWindow
 */
void MainWindow::handleUpdateClusterWindow(bool)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            if (cluster.status() == Cluster::Status::Disconnected)
            {
                NewClusterWindow* win = new NewClusterWindow;
                connect(win, SIGNAL(completeUpdateCluster(Cluster)), this, SLOT(handleUpdateTreeItem(Cluster)));
                connect(this, & MainWindow::activateNewClusterWindow, win, & NewClusterWindow::afterWindowActivated);
                emit activateNewClusterWindow(&cluster);
                win->exec();
            }
            else
            {
                QMessageBox::warning(this, "Warning", "Please disconnect the Cluster before modify Cluster.");
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "The Cluster must be selected at first.");
    }
}

void MainWindow::handleNewTenantWindow(bool)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            TenantWindow* win = new TenantWindow;
            connect(win, SIGNAL(completedNewTenant(Tenant)), this, SLOT(handleInsertTreeItem(Tenant)));
            connect(this, & MainWindow::activateNewTenantWindow, win, & TenantWindow::afterWindowActivated);
            emit activateNewTenantWindow(cluster);
            win->exec();
        }
    }
}

void MainWindow::handleNewNamespaceWindow(bool)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Tenant>())
    {
        NamespaceWindow* win = new NamespaceWindow;
        Tenant tenant = data.value<Tenant>();
        connect(win, SIGNAL(completedNewNamespace(Namespace)), this, SLOT(handleInsertTreeItem(Namespace)));
        connect(this, & MainWindow::activateNewNamespaceWindow, win, & NamespaceWindow::afterWindowActivated);
        emit activateNewNamespaceWindow(tenant);
        win->exec();
    }
}

void MainWindow::handleNewTokenWindow(bool)
{
    TokenWindow* win = new TokenWindow;
    win->exec();
}

void MainWindow::handleFunctionWindow(bool)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Namespace>())
    {
        Namespace ns = data.value<Namespace>();
        MdiSubWindow* subWindow = findMdiSubWindow(MdiSubWindow::SubWindowType::FunctionSubWindow);
        if (subWindow)
        {
            this->mdiMain->setActiveSubWindow(subWindow);
        }
        else
        {
            BaseMdiSubWindow* mdiChild = new FunctionsWindow(this->mdiMain);
            this->mdiMain->addSubWindow(mdiChild);
            connect(this, & MainWindow::activateFunctionSubWindow, mdiChild, & BaseMdiSubWindow::afterWindowActivated);
        }
        emit activateFunctionSubWindow(QVariant::fromValue(ns));
    }
}

void MainWindow::handleSourceWindow(bool)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Namespace>())
    {
        Namespace ns = data.value<Namespace>();
        MdiSubWindow* subWindow = findMdiSubWindow(MdiSubWindow::SubWindowType::SourceSubWindow);
        if (subWindow)
        {
            this->mdiMain->setActiveSubWindow(subWindow);
        }
        else
        {
            BaseMdiSubWindow* mdiChild = new SourcesWindow(this->mdiMain);
            this->mdiMain->addSubWindow(mdiChild);
            connect(this, & MainWindow::activateSourceSubWindow, mdiChild, & BaseMdiSubWindow::afterWindowActivated);
            //mdiChild->showMaximized();
        }
        emit activateSourceSubWindow(QVariant::fromValue(ns));
    }
}

void MainWindow::handleSinkWindow(bool)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Namespace>())
    {
        Namespace ns = data.value<Namespace>();
        MdiSubWindow* subWindow = findMdiSubWindow(MdiSubWindow::SubWindowType::SinkSubWindow);
        if (subWindow)
        {
            this->mdiMain->setActiveSubWindow(subWindow);
        }
        else
        {
            BaseMdiSubWindow* mdiChild = new SinksWindow(this->mdiMain);
            this->mdiMain->addSubWindow(mdiChild);
            connect(this, & MainWindow::activateSinkSubWindow, mdiChild, & BaseMdiSubWindow::afterWindowActivated);
            mdiChild->showMaximized();
        }
        emit activateSinkSubWindow(QVariant::fromValue(ns));
    }
}

void MainWindow::handlePermissionWindow(bool)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Namespace>())
    {
        Namespace ns = data.value<Namespace>();
        PermissionsWindow* win = new PermissionsWindow;
        connect(this, & MainWindow::activatePermissionWindow, win, & PermissionsWindow::afterWindowActivated);
        emit activatePermissionWindow(ns);
        win->exec();
    }
}

void MainWindow::handleOpenCluster(bool)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            if (cluster.status() == Cluster::Status::Disconnected)
            {
                handleTreeItemDoubleClicked(item, 0);
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "The Cluster must be selected at first.");
    }
}

void MainWindow::handleCloseCluster(bool)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            if (cluster.status() == Cluster::Status::Connected)
            {
                cluster.setStatus(Cluster::Status::Disconnected);
                item->setIcon(0, QIcon(":/disconnected"));
                this->tlbTenantToolbar->setEnabled(false);
                this->mdiMain->closeAllSubWindows();
                this->treeTenants->clear();
                item->setData(0, Qt::UserRole, QVariant::fromValue(cluster));
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "The Cluster must be selected at first.");
    }
}

void MainWindow::handleInsertTreeItem(const Cluster& _cluster)
{
    if (!this->m_ClusterService->exist(_cluster.name()))
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(this->treeClusters);
        item->setText(0, _cluster.name());
        item->setIcon(0, QIcon(":/disconnected"));
        item->setData(0, Qt::UserRole, QVariant::fromValue(_cluster));
        this->treeClusters->insertTopLevelItem(0, item);
        this->m_ClusterService->addCluster(_cluster);
    }
}

void MainWindow::handleInsertTreeItem(const Tenant& _tenant)
{
    QTreeWidgetItem* item = new  QTreeWidgetItem(this->treeTenants);
    item->setText(0, _tenant.name());
    item->setIcon(0, QIcon(":/tenant"));
    item->setData(0, Qt::UserRole, QVariant::fromValue(_tenant));
    this->treeTenants->insertTopLevelItem(0, item);
}

void MainWindow::handleInsertTreeItem(const Namespace& _namespace)
{
    QTreeWidgetItem* item = this->treeTenants->currentItem();
    QVariant data = item->data(0, Qt::UserRole);
    if (data.canConvert<Tenant>())
    {
        QTreeWidgetItem* subitem = new QTreeWidgetItem(item);
        subitem->setText(0, _namespace.name());
        subitem->setIcon(0, QIcon(":/namespace"));
        subitem->setData(0, Qt::UserRole, QVariant::fromValue(_namespace));
    }
}

void MainWindow::handleUpdateTreeItem(const Cluster& _cluster)
{
    QTreeWidgetItem* item = this->treeClusters->currentItem();
    if (item)
    {
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            Cluster cluster = data.value<Cluster>();
            cluster = _cluster;
            item->setData(0, Qt::UserRole, QVariant::fromValue(cluster));
            this->m_ClusterService->modifyCluster(cluster);
        }
    }
}

void MainWindow::handleTreeItemDoubleClicked(QTreeWidgetItem* _item, int)
{
    QVariant data = _item->data(0, Qt::UserRole);
    if (data.canConvert<Cluster>())
    {
        Cluster cluster = data.value<Cluster>();
        if (cluster.status() == Cluster::Status::Disconnected)
        {
            this->mdiMain->closeAllSubWindows();
            cluster.setStatus(Cluster::Status::Connected);
            _item->setIcon(0, QIcon(":/connected"));
            _item->setData(0, Qt::UserRole, QVariant::fromValue(cluster));
            this->tlbTenantToolbar->setEnabled(true);
            handleLoadTenantsAndNamespaces(cluster);
        }
    }
    else if (data.canConvert<Namespace>())
    {
        Namespace ns = data.value<Namespace>();
        MdiSubWindow* subWindow = findMdiSubWindow(MdiSubWindow::SubWindowType::TopicSubWindow);
        if (subWindow)
        {
            this->mdiMain->setActiveSubWindow(subWindow);
        }
        else
        {
            TopicsWindow* mdiChild = new TopicsWindow(this->mdiMain);
            this->mdiMain->addSubWindow(mdiChild);
            connect(this, & MainWindow::activateTopicSubWindow, mdiChild, & TopicsWindow::afterWindowActivated);

            //mdiChild->showMaximized();
        }
        emit activateTopicSubWindow(QVariant::fromValue(ns));
    }
}

void MainWindow::handleTreeItemPressed(QTreeWidgetItem* _item, int)
{
    if (qApp->mouseButtons() == Qt::RightButton)
    {
        QMenu* popup = new QMenu(this);
        QVariant data = _item->data(0, Qt::UserRole);
        if (data.canConvert<Cluster>())
        {
            popup->addAction(this->actEditCluster);
            popup->addAction(this->actDelCluster);
            popup->addSeparator();
            popup->addAction(this->actOpenCluster);
            popup->addAction(this->actCloseCluster);
        }
        else if (data.canConvert<Tenant>())
        {
            popup->addAction(this->actDelTenant);
            popup->addSeparator();
            popup->addAction(this->actNewNamespace);
        }
        else if (data.canConvert<Namespace>())
        {
            popup->addAction(this->actPermission);
            popup->addSeparator();
            popup->addAction(this->actDelNamespace);
            popup->addSeparator();
            popup->addAction(this->actFunctions);
            popup->addAction(this->actSinks);
            popup->addAction(this->actSources);
            Namespace ns = data.value<Namespace>();
            if (ns.tenant().cluster().hasFunctionUrl())
            {
                this->actFunctions->setEnabled(true);
                this->actSinks->setEnabled(true);
                this->actSources->setEnabled(true);
            }
            else
            {
                this->actFunctions->setEnabled(false);
                this->actSinks->setEnabled(false);
                this->actSources->setEnabled(false);
            }

        }
        popup->exec(QCursor::pos());
    }
}

void MainWindow::handleTreeCurrentItemChanged(QTreeWidgetItem* _current, QTreeWidgetItem* _previous)
{
    Q_UNUSED(_previous);
    QVariant data = _current->data(0, Qt::UserRole);
    if (data.canConvert<Cluster>())
    {
        Cluster cluster = data.value<Cluster>();
        if (cluster.status() == Cluster::Status::Connected)
        {
            handleLoadTenantsAndNamespaces(cluster);
            this->tlbTenantToolbar->setEnabled(true);
        }
        else
        {
            this->tlbTenantToolbar->setEnabled(false);
            this->mdiMain->closeAllSubWindows();
            this->treeTenants->clear();
        }
    }
}

void MainWindow::handleDeleteCluster(bool)
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Cluster"), tr("Would you like to remove this Cluster?"));
    if (button == QMessageBox::Yes)
    {
        QTreeWidgetItem* item = this->treeClusters->currentItem();
        if (item)
        {
            QVariant data = item->data(0, Qt::UserRole);
            if (data.canConvert<Cluster>())
            {
                Cluster cluster = data.value<Cluster>();
                if (cluster.status() == Cluster::Status::Connected)
                {
                    this->tlbTenantToolbar->setEnabled(false);
                    this->mdiMain->closeAllSubWindows();
                    this->treeTenants->clear();
                }
                this->m_ClusterService->removeCluster(cluster);
                delete item;
            }
        }
        else
        {
            QMessageBox::warning(this, "Warning", "The Cluster must be selected at first.");
        }
    }
}

void MainWindow::handleDeleteTenant(bool)
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Tenant"), tr("Are you sure you want to delete this Tenant?"));
    if (button == QMessageBox::Yes)
    {
        QTreeWidgetItem* item = this->treeTenants->currentItem();
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Tenant>())
        {
            Tenant tenant = data.value<Tenant>();
            HttpStatusCode error;
            this->m_TenantService->deleteTenant(tenant, error);
            if (error.code == HttpStatusCode::StatusCode::NoContent)
            {
                delete item;
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
    }
}

void MainWindow::handleDeleteNamespace(bool)
{
    QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Namespace"), tr("Are you sure you want to delete this Namespace?"));
    if (button == QMessageBox::Yes)
    {
        QTreeWidgetItem* item = this->treeTenants->currentItem();
        QVariant data = item->data(0, Qt::UserRole);
        if (data.canConvert<Namespace>())
        {
            HttpStatusCode error;
            this->m_NamespaceService->deleteNamespace(data.value<Namespace>(), error);
            if (error.code == HttpStatusCode::StatusCode::NoContent)
            {
                delete item;
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
    }
}

void MainWindow::updateMenus()
{
    bool hasMdiChild = this->mdiMain->activeSubWindow() != nullptr;
    this->closeAct->setEnabled(hasMdiChild);
    this->closeAllAct->setEnabled(hasMdiChild);
    this->tileAct->setEnabled(hasMdiChild);
    this->cascadeAct->setEnabled(hasMdiChild);
    this->nextAct->setEnabled(hasMdiChild);
    this->previousAct->setEnabled(hasMdiChild);
    this-> windowMenuSeparatorAct->setVisible(hasMdiChild);
}

void MainWindow::updateWindowMenu()
{
    this->menuWindow->clear();
    this->menuWindow->addAction(this->closeAct);
    this->menuWindow->addAction(this->closeAllAct);
    this->menuWindow->addSeparator();
    this->menuWindow->addAction(this->tileAct);
    this->menuWindow->addAction(this->cascadeAct);
    this->menuWindow->addSeparator();
    this->menuWindow->addAction(this->nextAct);
    this->menuWindow->addAction(this->previousAct);
    this->menuWindow->addAction(this->windowMenuSeparatorAct);

    QList<QMdiSubWindow*> windows = this->mdiMain->subWindowList();
    windowMenuSeparatorAct->setVisible(!windows.isEmpty());

    for (int i = 0; i < windows.size(); ++i)
    {
        MdiSubWindow* child = qobject_cast<MdiSubWindow*>(windows.at(i));

        QString text(tr("%1 Window"));
        QAction* action = this->menuWindow->addAction(text.arg(child->windowTitle()), child, [this, child]()
        {
            this->mdiMain->setActiveSubWindow(child);
        });
        action->setCheckable(true);
        action ->setChecked(child == this->mdiMain->activeSubWindow());
    }
}
