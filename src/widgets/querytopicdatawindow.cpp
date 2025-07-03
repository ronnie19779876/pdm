#include "querytopicdatawindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QIcon>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QClipboard>

#include "../services/prestoqueryservice.h"
#include "../table.h"
#include "../topic.h"

QueryTopicDataWindow::QueryTopicDataWindow(QWidget* parent) : QDialog(parent), twResult(new QTableWidget(this)), meuPopupMenu(new QMenu(this)), m_Query(new PrestoQueryService(this)), m_Statement(new Statement())
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblTopicName = new QLabel;
    this->teCondition = new QTextEdit;
    this->teCondition->setFixedHeight(50);
    formLayout->addRow(tr("&Topic Name:"), this->lblTopicName);
    formLayout->addRow(new QLabel("Query Condition:"));
    formLayout->addRow(this->teCondition);

    //this->twResult->setColumnCount(header.length());
    //this->twResult->setHorizontalHeaderLabels(header);
    this->twResult->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    this->twResult->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twResult->horizontalHeader()->setStretchLastSection(true);
    this->twResult->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twResult->verticalHeader()->setHidden(true);
    this->twResult->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twResult->setFocusPolicy(Qt::NoFocus);
    this->twResult->setContextMenuPolicy(Qt::CustomContextMenu);
    formLayout->addRow(new QLabel("Query Result:"));
    QHBoxLayout* actionsLayout = new QHBoxLayout;
    this->btnStop = new QPushButton(QIcon(":/stop"), tr("&Stop"));
    this->btnStop->setEnabled(false);
    actionsLayout->addWidget(this->btnStop);
    actionsLayout->addStretch();
    formLayout->addRow(actionsLayout);
    formLayout->addRow(this->twResult);

    this->lblStatus = new QLabel("Message: ");
    formLayout->addRow(this->lblStatus);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnQuery = new QPushButton(tr("&Query"));
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnQuery = new QPushButton(okIcon, tr("&Query"));
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(this->btnQuery);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(btnCancel);

    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    //setFixedWidth(900);
    //setFixedHeight(540);
    this->resize(1024, 540);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Query Topic Data"));
    setWindowIcon(QIcon(":/sql"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(btnCancel, &QPushButton::clicked, this, &QueryTopicDataWindow::close);
    connect(this->btnQuery, &QPushButton::clicked, this, &QueryTopicDataWindow::handleQuery);
    connect(this->btnStop, &QPushButton::clicked, this, &QueryTopicDataWindow::handleCancelQuery);
    connect(this, &QueryTopicDataWindow::queryNext, this, &QueryTopicDataWindow::handleQueryNext);
    connect(this, &QueryTopicDataWindow::running, this, &QueryTopicDataWindow::handleRunning);
    connect(this, &QueryTopicDataWindow::finish, this, &QueryTopicDataWindow::handleFinish);
    connect(this, &QueryTopicDataWindow::error, this, &QueryTopicDataWindow::handleError);
    connect(this->twResult, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleTableContextMenu(QPoint)));

    QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/images/copy.ico"));
    this->actCopyCellText = new QAction(copyIcon, tr("&Copy Cell"));
    this->actCopyRowText = new QAction(tr("&Copy Row"));
    this->meuPopupMenu->addAction(this->actCopyCellText);
    this->meuPopupMenu->addAction(this->actCopyRowText);
    connect(this->actCopyCellText, &QAction::triggered, this, &QueryTopicDataWindow::handleCopyCellText);
    connect(this->actCopyRowText, &QAction::triggered, this, &QueryTopicDataWindow::handleCopyRowText);
}

QueryTopicDataWindow::~QueryTopicDataWindow()
{
    if (m_Statement)
    {
        delete m_Statement;
        m_Statement = nullptr;
    }
}

void QueryTopicDataWindow::afterWindowActivated(const QVariant& _var)
{
    this->m_Variant = _var;
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        this->lblTopicName->setText(QString("%1://%2/%3/%4").arg(topic.domain(), topic.getNamespace().tenant().name(), topic.getNamespace().name(), topic.name()));
        if (!topic.authToken().isEmpty())
        {
            this->m_Query->setAuthToken(topic.authToken());
        }
    }
}

void QueryTopicDataWindow::handleQuery()
{
    Topic topic = this->m_Variant.value<Topic>();
    if (topic.getNamespace().tenant().cluster().hasPrestoUrl())
    {
        m_Statement->reset();
        this->btnQuery->setEnabled(false);
        m_Statement->setCondition(this->teCondition->toPlainText());
        this->m_Query->query(topic, m_Statement);
        emit queryNext();
    }
    else
    {
        QMessageBox::warning(this, "Warning", "Presto Service has not allowed.");
    }
}

void QueryTopicDataWindow::handleQueryNext()
{
    this->m_Query->queryNext(m_Statement);
    QueryState state = m_Statement->state();
    QString status("Query %1, %2, %3ms [%4 rows, %5B]");
    status = status.arg(m_Statement->id(), state.state()).arg(state.elapsedTimeMillis()).arg(state.processedRows()).arg(state.processedBytes());
    this->lblStatus->setText(status);
    if (state.state() == QString("QUEUED") || state.state() == QString("PLANNING"))
    {
        emit queryNext();
    }
    else if (state.state() == QString("RUNNING"))
    {
        emit running();
    }
    else if (state.state() == QString("FINISHED") || state.state() == QString("CANCELED"))
    {
        emit finish();
    }
    else if (state.state() == QString("FAILED"))
    {
        emit error();
    }
}

void QueryTopicDataWindow::handleRunning()
{
    this->btnStop->setEnabled(true);
    emit queryNext();
}

void QueryTopicDataWindow::handleFinish()
{
    QList<Column> columns = m_Statement->columns();
    QStringList header;
    foreach (const Column& column, columns)
    {
        header << column.name();
    }
    this->twResult->setColumnCount(header.length());
    this->twResult->setHorizontalHeaderLabels(header);
    this->twResult->clearContents();
    QStringList rows = m_Statement->result();
    this->twResult->setRowCount(rows.length());
    for (int i = 0, n = rows.size(); i < n; ++i)
    {
        QStringList data = rows[i].split("\t");
        for (int j = 0, m = data.size(); j < m; ++j)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[j]);
            this->twResult->setItem(i, j, item);
            item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
        }
    }
    this->btnStop->setEnabled(false);
    this->btnQuery->setEnabled(true);

}

void QueryTopicDataWindow::handleError()
{
    QMessageBox::critical(this, tr("Error"), m_Statement->error());
    this->btnStop->setEnabled(false);
    this->btnQuery->setEnabled(true);
    m_Statement->setError(QString());
}

void QueryTopicDataWindow::handleCancelQuery()
{
    this->btnStop->setEnabled(false);
    this->m_Query->cancelQuery(m_Statement);
}

void QueryTopicDataWindow::handleTableContextMenu(const QPoint&)
{
    this->meuPopupMenu->exec(QCursor::pos());
}

void QueryTopicDataWindow::handleCopyRowText(bool)
{
    QTableWidgetItem* item = this->twResult->currentItem();
    if (item && item->isSelected())
    {
        QString text;
        int row = item->row();
        int columns = this->twResult->columnCount();
        for (int i = 0, n = columns; i < n; ++i)
        {
            QTableWidgetItem* cell = this->twResult->item(row, i);
            text.append(cell->text()).append('\t');
        }
        QClipboard* board = QApplication::clipboard();
        board->setText(text);
    }
}

void QueryTopicDataWindow::handleCopyCellText(bool)
{
    QTableWidgetItem* item = this->twResult->currentItem();
    if (item && item->isSelected())
    {
        QClipboard* board = QApplication::clipboard();
        board->setText(item->text());
    }
}
