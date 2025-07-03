#include "topicoverviewwindow.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QComboBox>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QAction>
#include <QMenu>
#include <QApplication>
#include <QClipboard>
#include <QMessageBox>

#include "../services/topicservice.h"
#include "../widgets/subscriptionwindow.h"
#include "../widgets/peekmessageswindow.h"

TopicOverviewWindow::TopicOverviewWindow(QWidget* parent) : QDialog(parent), m_TopicService(new TopicService(this)), twProducers(new QTableWidget(this)), twSubscriptions(new QTableWidget(this)), twConsumers(new QTableWidget(this)), twCurrent(nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblTopicName = new QLabel;
    this->cbPartitions = new QComboBox;
    formLayout->addRow(tr("&Topic Name:"), this->lblTopicName);
    formLayout->addRow(tr("Partition:"), this->cbPartitions);
    layout->addLayout(formLayout);
    layout->addWidget(new QLabel(tr("Producers:")));

    QStringList header;
    header << tr("Producer Id") << tr("Producer Name") << tr("In Rate") << tr("In Throughout") << tr("Avg Msg Size") << tr("Address") << tr("Since");
    this->twProducers->setColumnCount(header.length());
    this->twProducers->setHorizontalHeaderLabels(header);
    this->twProducers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twProducers->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twProducers->horizontalHeader()->setStretchLastSection(true);
    this->twProducers->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twProducers->verticalHeader()->setHidden(true);
    this->twProducers->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twProducers->setFocusPolicy(Qt::NoFocus);
    this->twProducers->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twProducers);

    //layout->addWidget(new QLabel(tr("Subscriptions:")));
    QHBoxLayout* actionsLayout = new QHBoxLayout;
    this->btnNewSubsription = new QPushButton(tr("New &Subscription"));
    actionsLayout->addWidget(new QLabel(tr("Subscriptions:")));
    actionsLayout->addStretch();
    actionsLayout->addWidget(this->btnNewSubsription);
    layout->addLayout(actionsLayout);
    header.clear();
    header << tr("Subscription Name") << tr("Type") << tr("Out Rate") << tr("Out Throughout") << tr("Msg Expired") << tr("Backlog");
    this->twSubscriptions->setColumnCount(header.length());
    this->twSubscriptions->setHorizontalHeaderLabels(header);
    this->twSubscriptions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twSubscriptions->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twSubscriptions->horizontalHeader()->setStretchLastSection(true);
    this->twSubscriptions->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twSubscriptions->verticalHeader()->setHidden(true);
    this->twSubscriptions->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twSubscriptions->setFocusPolicy(Qt::NoFocus);
    this->twSubscriptions->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twSubscriptions);

    layout->addWidget(new QLabel(tr("Consumers:")));
    header.clear();
    header << tr("Consumer Name") << tr("Out Rate") << tr("Out Throughout") << tr("Avg Msg Size") << tr("Address") << tr("Since");
    this->twConsumers->setColumnCount(header.length());
    this->twConsumers->setHorizontalHeaderLabels(header);
    this->twConsumers->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twConsumers->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twConsumers->horizontalHeader()->setStretchLastSection(true);
    this->twConsumers->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twConsumers->verticalHeader()->setHidden(true);
    this->twConsumers->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twConsumers->setFocusPolicy(Qt::NoFocus);
    this->twConsumers->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twConsumers);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnCancel);
    layout->addLayout(buttonLayout);

    createActions();

    setLayout(layout);
    this->resize(1024, 680);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Topic Overview"));
    setWindowIcon(QIcon(":/overview"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(btnCancel, &QPushButton::clicked, this, &TopicOverviewWindow::close);
    connect(this, &TopicOverviewWindow::initialize, this, &TopicOverviewWindow::handleInitialize);
    connect(this->cbPartitions, &QComboBox::currentTextChanged, this, &TopicOverviewWindow::handleCurrentIndexChanged);
    connect(this->twSubscriptions, &QTableWidget::itemDoubleClicked, this, &TopicOverviewWindow::handleItemDoubleClicked);
    connect(this, SIGNAL(load(Topic, int)), this, SLOT(handleLoad(Topic, int)));
    connect(this, SIGNAL(load(Subscription)), this, SLOT(handleLoad(Subscription)));
    connect(this->twProducers, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleProducerTableContextMenu(QPoint)));
    connect(this->twSubscriptions, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleConsumerTableContextMenu(QPoint)));
    connect(this->btnNewSubsription, &QPushButton::clicked, this, &TopicOverviewWindow::handleActivateNewSubscriptionWindow);
}

void TopicOverviewWindow::afterWindowActivated(const QVariant& _var)
{
    this->m_Variant = _var;
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        if (!topic.authToken().isEmpty())
        {
            this->m_TopicService->setAuthToken(topic.authToken());
        }
    }
    emit initialize();
}

void TopicOverviewWindow::createActions()
{
    this->actCopyCellText = new QAction(QIcon(":/copy-cell"), tr("&Copy Cell"));
    this->actCopyRowText = new QAction(QIcon(":/copy-row"), tr("&Copy Row"));
    this->meuProducers = new QMenu(this);
    this->meuProducers->addAction(this->actCopyCellText);
    this->meuProducers->addAction(this->actCopyRowText);

    this->actPeekMsg = new QAction(QIcon(":/message"), tr("&Peek nth Messages..."));
    this->actPeekMsg->setToolTip(tr("Peek nth message on a topic subscription."));
    this->actRemoveSubscription = new QAction(QIcon(":/remove"), tr("&Delete Subscription"));
    this->actRemoveSubscription->setToolTip(tr("Delete a subscription."));
    this->meuConsumers = new QMenu(this);
    this->meuConsumers->addAction(this->actRemoveSubscription);
    this->meuConsumers->addSeparator();
    this->meuConsumers->addAction(this->actPeekMsg);
    this->meuConsumers->addSeparator();
    this->meuConsumers->addAction(this->actCopyCellText);
    this->meuConsumers->addAction(this->actCopyRowText);

    connect(this->actCopyCellText, &QAction::triggered, this, &TopicOverviewWindow::handleCopyCellText);
    connect(this->actCopyRowText, &QAction::triggered, this, &TopicOverviewWindow::handleCopyRowText);
    connect(this->actPeekMsg, &QAction::triggered, this, &TopicOverviewWindow::handleActivatePeekMessagesWindow);
    connect(this->actRemoveSubscription, &QAction::triggered, this, &TopicOverviewWindow::handleDeleteSubscription);

}

void TopicOverviewWindow::handleInitialize()
{
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        this->lblTopicName->setText(QString("%1://%2/%3/%4").arg(topic.domain(), topic.getNamespace().tenant().name(), topic.getNamespace().name(), topic.name()));

        int partitions = topic.stats().partitions();
        if (partitions > 0)
        {
            for (int i = 0, n = partitions; i < n; i++)
            {
                this->cbPartitions->addItem(QString::number(i));
            }
        }
        else
        {
            handleCurrentIndexChanged(QString());
        }
    }
}

void TopicOverviewWindow::handleCurrentIndexChanged(const QString&)
{
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
        emit load(topic, partitions);
    }
}

void TopicOverviewWindow::handleLoad(const Topic& _topic, const int& _partitions)
{
    TopicStats stats = this->m_TopicService->overview(_topic, _partitions);
    this->twProducers->clearContents();
    QList<Producer> producers = stats.publishers();
    if (producers.length() > 0)
    {
        this->twProducers->setRowCount(producers.size());
        QStringList columns;
        columns << "id" << "name" << "inRate"  << "inThroughout" << "avgMsgSize" << "address" << "since";
        for (int i = 0, n = producers.length(); i < n; ++i)
        {
            ProducerData data = producers[i].toData();
            for (int j = 0, m = columns.length(); j < m; j++)
            {
                QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
                this->twProducers->setItem(i, j, item);
                if (j == 0)
                    item->setData(Qt::UserRole, QVariant::fromValue(producers[i]));
            }
        }
    }

    this->twSubscriptions->clearContents();
    QList<Subscription> subscriptions = stats.subscriptions();
    if (subscriptions.length() > 0)
    {
        this->twSubscriptions->setRowCount(subscriptions.size());
        QStringList columns;
        columns << "name" << "type"  << "outRate" << "outThroughput" << "msgExpired" << "backlog";
        for (int i = 0, n = subscriptions.length(); i < n; ++i)
        {
            SubscriptionData data = subscriptions[i].toData();
            for (int j = 0, m = columns.length(); j < m; j++)
            {
                QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
                this->twSubscriptions->setItem(i, j, item);
                if (j == 0)
                    item->setData(Qt::UserRole, QVariant::fromValue(subscriptions[i]));
            }
        }
    }
}

void TopicOverviewWindow::handleLoad(const Subscription& _subscription)
{
    this->twConsumers->clearContents();
    QList<Consumer> consumers = _subscription.consumers();
    this->twConsumers->setRowCount(consumers.size());
    if (consumers.size() > 0)
    {
        QStringList columns;
        columns << "name" << "outRate" << "outThroughput" << "avgMsgSize" << "address" << "since";
        for (int i = 0, n = consumers.length(); i < n; ++i)
        {
            ConsumerData data = consumers[i].toData();
            for (int j = 0, m = columns.length(); j < m; j++)
            {
                QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
                this->twConsumers->setItem(i, j, item);
                if (j == 0)
                    item->setData(Qt::UserRole, QVariant::fromValue(consumers[i]));
            }
        }
    }
}

void TopicOverviewWindow::handleItemDoubleClicked(QTableWidgetItem* _item)
{
    int row = _item->row();
    QTableWidgetItem* first = this->twSubscriptions->item(row, 0);
    QVariant data = first->data(Qt::UserRole);
    if (data.canConvert<Subscription>())
    {
        Subscription subscription = data.value<Subscription>();
        emit load(subscription);
    }
}

void TopicOverviewWindow::handleProducerTableContextMenu(const QPoint&)
{
    this->twCurrent = qobject_cast<QTableWidget*>(sender());
    this->meuProducers->exec(QCursor::pos());
}

void TopicOverviewWindow::handleConsumerTableContextMenu(const QPoint&)
{
    this->twCurrent = qobject_cast<QTableWidget*>(sender());
    this->meuConsumers->exec(QCursor::pos());
}

void TopicOverviewWindow::handleCopyCellText(bool)
{
    if (this->twCurrent)
    {
        QTableWidgetItem* item = this->twCurrent->currentItem();
        if (item && item->isSelected())
        {
            QClipboard* board = QApplication::clipboard();
            board->setText(item->text());
        }
    }
    this->twCurrent = nullptr;
}

void TopicOverviewWindow::handleCopyRowText(bool)
{
    if (this->twCurrent)
    {
        QTableWidgetItem* item = this->twCurrent->currentItem();
        if (item && item->isSelected())
        {
            QString text;
            int row = item->row();
            int columns = this->twCurrent->columnCount();
            for (int i = 0, n = columns; i < n; ++i)
            {
                QTableWidgetItem* cell = this->twCurrent->item(row, i);
                text.append(cell->text()).append('\t');
            }
            QClipboard* board = QApplication::clipboard();
            board->setText(text);
        }
    }
    this->twCurrent = nullptr;
}

void TopicOverviewWindow::handleActivatePeekMessagesWindow(bool)
{
    QTableWidgetItem* current = this->twSubscriptions->currentItem();
    if (current)
    {
        int row = current->row();
        QTableWidgetItem* item = this->twSubscriptions->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        Subscription sub = data.value<Subscription>();
        Topic topic = this->m_Variant.value<Topic>();
        int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
        PeekMessagesWindow* win = new PeekMessagesWindow;
        connect(this, &TopicOverviewWindow::activatePeekNthMessagesWindow, win, &PeekMessagesWindow::afterWindowActivated);
        emit activatePeekNthMessagesWindow(topic, sub, partitions);
        win->exec();
    }
}

void TopicOverviewWindow::handleActivateNewSubscriptionWindow(bool)
{
    Topic topic = this->m_Variant.value<Topic>();
    SubscriptionWindow* win = new  SubscriptionWindow;
    connect(this, &TopicOverviewWindow::activateNewSubscriptionWindow, win, &SubscriptionWindow::afterWindowActivated);
    connect(win, &SubscriptionWindow::completedNewSubscription, this, &TopicOverviewWindow::handleInitialize);
    emit activateNewSubscriptionWindow(QVariant::fromValue(topic));
    win->exec();
}

void TopicOverviewWindow::handleDeleteSubscription(bool)
{
    QTableWidgetItem* current = this->twSubscriptions->currentItem();
    if (current)
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Subscription"), tr("Are you sure you want to delete this subscription?"));
        if (button == QMessageBox::Yes)
        {
            int row = current->row();
            QTableWidgetItem* item = this->twSubscriptions->item(row, 0);
            QVariant data = item->data(Qt::UserRole);
            Subscription sub = data.value<Subscription>();
            Topic topic = this->m_Variant.value<Topic>();
            HttpStatusCode error;
            this->m_TopicService->deleteSubscription(topic, sub.name(), error);
            if (error.code == HttpStatusCode::StatusCode::NoContent)
            {
                int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
                emit load(topic, partitions);
            }
            else
            {
                QMessageBox::critical(this, tr("Error"), error.errorString());
            }
        }
    }
}
