#include "functioninstanceswindow.h"

#include <QVBoxLayout>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

#include "../functioninstance.h"
#include "../services/functionservice.h"
#include "../services/sinkservice.h"
#include "../services/sourceservice.h"

FunctionInstancesWindow::FunctionInstancesWindow(QWidget* parent) : QDialog(parent), m_SourceService(new SourceService(this)), m_SinkService(new SinkService(this)), m_FunctionService(new FunctionService(this)), twInstances(new QTableWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setContentsMargins(5, 5, 5, 5);
    layout->setSpacing(1);

    this->twInstances->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twInstances->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twInstances->horizontalHeader()->setStretchLastSection(true);
    this->twInstances->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twInstances->verticalHeader()->setHidden(true);
    this->twInstances->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twInstances->setFocusPolicy(Qt::NoFocus);
    this->twInstances->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twInstances);

    QHBoxLayout* btnLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
#endif
    btnLayout->setContentsMargins(0, 0, 0, 0);
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);

    layout->addLayout(btnLayout);

    setLayout(layout);
    resize(QSize(950, 340));
    //setFixedSize(QSize(950, 340));
    setWindowIcon(QIcon(":/overview"));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(btnCancel, &QPushButton::clicked, this, &FunctionInstancesWindow::close);
    this->m_Columns  << "tenant" << "namesapces" << "name"  << "id" << "workId" << "status" << "received" << "written" << "errors";
}

void FunctionInstancesWindow::afterWindowActivated(const QVariant& _var)
{
    QStringList header;
    header << tr("Tenant") << tr("Namespace") << tr("Name") << tr("InstanceId") << tr("WorkId") << tr("Running") << tr("Received") << tr("Written") << tr("Errors");
    this->twInstances->setColumnCount(header.length());
    this->twInstances->setHorizontalHeaderLabels(header);
    this->m_Variant = _var;
    if (_var.canConvert<Sink>())
    {
        setWindowTitle(tr("Sink Instances"));
    }
    else if (_var.canConvert<Source>())
    {
        setWindowTitle(tr("Source Instances"));
    }
    else if (_var.canConvert<Function>())
    {
        setWindowTitle(tr("Function Instances"));
    }
    connect(this, &FunctionInstancesWindow::initialize, this, &FunctionInstancesWindow::handleInitialize);
    emit initialize(this->m_Variant);
}

void FunctionInstancesWindow::handleInitialize(const QVariant& _var)
{
    if (_var.canConvert<Sink>())
    {
        Sink sink = _var.value<Sink>();
        handleLoadSinkInstances(sink);
    }
    else if (_var.canConvert<Source>())
    {
        Source source = _var.value<Source>();
        handleLoadSourceInstances(source);
    }
    else if (_var.canConvert<Function>())
    {
        Function function = _var.value<Function>();
        handleLoadFunctionInstances(function);
    }
}

void FunctionInstancesWindow::handleLoadSinkInstances(const Sink& _sink)
{
    QList<FunctionInstance> instances = this->m_SinkService->instances(_sink);
    this->twInstances->clearContents();
    this->twInstances->setRowCount(instances.length());
    for (int i = 0, n = instances.length(); i < n ; i++)
    {
        FunctionInstanceData data = instances[i].toData();
        for (int j = 0, m = m_Columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[j]]);
            item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
            this->twInstances->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(instances[i]));
        }
    }
}

void FunctionInstancesWindow::handleLoadSourceInstances(const Source& _source)
{
    QList<SourceInstance> instances = this->m_SourceService->instances(_source);
    this->twInstances->clearContents();
    this->twInstances->setRowCount(instances.length());
    for (int i = 0, n = instances.length(); i < n ; i++)
    {
        SourceInstanceData data = instances[i].toData();
        for (int j = 0, m = m_Columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[j]]);
            item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
            this->twInstances->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(instances[i]));
        }
    }
}

void FunctionInstancesWindow::handleLoadFunctionInstances(const Function& _function)
{
    QList<FunctionInstance> instances = this->m_FunctionService->instances(_function);
    this->twInstances->clearContents();
    this->twInstances->setRowCount(instances.length());
    for (int i = 0, n = instances.length(); i < n ; i++)
    {
        FunctionInstanceData data = instances[i].toData();
        for (int j = 0, m = m_Columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[m_Columns[j]]);
            item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
            this->twInstances->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(instances[i]));
        }
    }
}
