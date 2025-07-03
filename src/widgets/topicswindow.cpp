#include "topicswindow.h"

#include <QVBoxLayout>
#include <QToolBar>
#include <QTableWidget>
#include <QHeaderView>
#include <QMessageBox>
#include <QApplication>
#include <QMenu>
#include <QClipboard>
#include <QDebug>

#include "../services/topicservice.h"
#include "../widgets/topicoverviewwindow.h"
#include "../widgets/topicstoragewindow.h"
#include "../widgets/newtopicwindow.h"
#include "../widgets/lastcommitmessagewindow.h"
#include "../widgets/querytopicdatawindow.h"
#include "../widgets/sendmessagewindow.h"

TopicsWindow::TopicsWindow(QWidget* _parent) : BaseMdiSubWindow(_parent), m_TopicService(new TopicService(this))
{
    this->actNew = new QAction(QIcon(":/addnew"), tr("&New Topic"), this);
    this->actNew->setStatusTip(tr("Create a new topic."));

    this->actDelete = new QAction(QIcon(":/remove"), tr("&Remove Topic"), this);
    this->actDelete->setStatusTip(tr("Delete a topic."));

    this->actRefresh = new QAction(QIcon(":/refresh"), QString("Refresh Topics"), this);
    this->actRefresh->setStatusTip(tr("Refresh topics."));

    this->actLastMessage = new QAction(QIcon(":/message"), tr("&Last Commit Message..."), this);
    this->actLastMessage->setStatusTip(tr("Get message by its messageId."));

    this->actQueryData = new QAction(QIcon(":/sql"), tr("&Query Topic Data..."), this);
    this->actQueryData->setStatusTip(tr("Query Avro data from a topic."));
    this->actQueryData->setEnabled(false);

    QIcon copyIcon = QIcon::fromTheme("edit-copy", QIcon(":/copy"));
    this->actCopy = new QAction(copyIcon, tr("&Copy Cell"), this);

    this->actStorage = new QAction(QIcon(":/storage"), tr("&Topic Storage..."), this);
    this->actStorage->setStatusTip(tr("Get the stored topic metadata."));

    this->actOverview = new QAction(QIcon(":/overview"), tr("&Topic Overview..."), this);
    this->actOverview->setStatusTip(tr("Overview the topic information."));

    this->actClose = new QAction(QIcon(":/exit"), tr("&Close"), this);
    this->actClose->setStatusTip(tr("Close the window."));

    this->actSendMessage = new QAction(QIcon(":/publish"), tr("&Send Message..."), this);
    this->actSendMessage->setStatusTip(tr("Send message by its topic."));

    this->tbToolbar->addAction(this->actNew);
    this->tbToolbar->addAction(this->actDelete);
    this->tbToolbar->addAction(this->actRefresh);
    this->tbToolbar->addSeparator();
    this->tbToolbar->addAction(this->actLastMessage);
    this->tbToolbar->addAction(this->actQueryData);
    this->tbToolbar->addAction(this->actStorage);
    this->tbToolbar->addAction(this->actSendMessage);
    this->tbToolbar->addSeparator();
    this->tbToolbar->addAction(this->actClose);

    connect(this->actNew, &QAction::triggered, this, &TopicsWindow::handleNewTopic);
    connect(this->actDelete, &QAction::triggered, this, &TopicsWindow::handleDeleteTopic);
    connect(this->actCopy, &QAction::triggered, this, &TopicsWindow::handleCopyCellText);
    connect(this->actRefresh, &QAction::triggered, this, &TopicsWindow::handleReload);
    connect(this->actLastMessage, &QAction::triggered, this, &TopicsWindow::handleLastCommitMessageWindow);
    connect(this->actQueryData, &QAction::triggered, this, &TopicsWindow::handleQueryTopicDataWindow);
    connect(this->actStorage, &QAction::triggered, this, &TopicsWindow::handleTopicStorageWindow);
    connect(this->actOverview, &QAction::triggered, this, &TopicsWindow::handleTopicOverviewWindow);
    connect(this->actSendMessage, &QAction::triggered, this, &TopicsWindow::handleSendMessageWindow);

    QStringList header;
    header << tr("Tenant") << tr("Namesapce") << tr("Topic") << tr("Partitions") << tr("Domian") << tr("Producers") << tr("Subscriptions");
    this->twTable->setColumnCount(header.length());
    this->twTable->setHorizontalHeaderLabels(header);

    setWindowTitle(tr("Topics"));
    setWindowIcon(QIcon(":/topic"));

    connect(this->twTable, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(handleTableContextMenu(QPoint)));
    connect(this, &TopicsWindow::removeTableItem, this, &TopicsWindow::handleRemoveTableItem);
    connect(this->twTable, &QTableWidget::itemPressed, this, &TopicsWindow::handleTableItemPressed);
    connect(this->twTable, &QTableWidget::itemDoubleClicked, this, &TopicsWindow::handleTableItemDoubleClicked);
    connect(this, &TopicsWindow::initialize, this, &TopicsWindow::handleReload);
    connect(this->actClose, &QAction::triggered, this, &TopicsWindow::close);
}


MdiSubWindow::SubWindowType TopicsWindow::subWindowType() const
{
    return MdiSubWindow::SubWindowType::TopicSubWindow;
}

void TopicsWindow::afterWindowActivated(const QVariant& _var)
{
    setVariant(_var);
    this->showMaximized();
    Namespace ns = value<Namespace>();
    if (ns.tenant().cluster().hasPrestoUrl())
    {
        this->actQueryData->setEnabled(true);
    }
    else
    {
        this->actQueryData->setEnabled(false);
    }
    if (!ns.authToken().isEmpty())
    {
        this->m_TopicService->setAuthToken(ns.authToken());
    }
    emit initialize();
}

bool TopicsWindow::existTopic(const Topic& _topic)
{
    bool result = false;
    //Does the new Topic already exist into table
    int row = this->twTable->rowCount();
    for (int i = 0, n = row; i < n; ++i)
    {
        QTableWidgetItem* item = this->twTable->item(i, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic other = data.value<Topic>();
            if (other == _topic)
            {
                result = true;
                break;
            }
        }
    }
    return result;
}

void TopicsWindow::handleReload()
{
    emit start();
    Namespace ns = value<Namespace>();
    QStringList columns;
    columns << "tenant" << "namesapces" << "name"  << "partitions" << "domain" << "producerNum" << "subscriptionNum";
    this->twTable->clearContents();
    QList<Topic> topics = this->m_TopicService->topics(ns);
    this->twTable->setRowCount(topics.length());
    for (int i = 0, n = topics.length(); i < n; i++)
    {
        TopicData data = topics[i].toData();
        for (int j = 0, m = columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
            item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
            this->twTable->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(topics[i]));
        }
    }
    this->twTable->repaint();
    emit stop();
}

void TopicsWindow::handleNewTopic(bool)
{
    NewTopicWindow* win = new NewTopicWindow;
    connect(this, &TopicsWindow::activateNewTopicWindow, win, &NewTopicWindow::afterWindowActivated);
    connect(win, &NewTopicWindow::completedNewTopic, this, &TopicsWindow::handleInsertTableItem);
    emit activateNewTopicWindow(this->variant());
    win->exec();
}

void TopicsWindow::handleLastCommitMessageWindow(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic topic = data.value<Topic>();
            LastCommitMessageWindow* win = new LastCommitMessageWindow;
            connect(this, &TopicsWindow::activateLastCommitMessageWindow, win, &LastCommitMessageWindow::afterWindowActivated);
            emit activateLastCommitMessageWindow(QVariant::fromValue(topic));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleTopicStorageWindow(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic topic = data.value<Topic>();
            TopicStorageWindow* win = new TopicStorageWindow;
            connect(this, &TopicsWindow::activateTopicStorageWindow, win, &TopicStorageWindow::afterWindowActivated);
            emit activateTopicStorageWindow(QVariant::fromValue(topic));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleQueryTopicDataWindow(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic topic = data.value<Topic>();
            QueryTopicDataWindow* win = new QueryTopicDataWindow;
            connect(this, &TopicsWindow::activateQueryTopicDataWindow, win, &QueryTopicDataWindow::afterWindowActivated);
            emit activateQueryTopicDataWindow(QVariant::fromValue(topic));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleTopicOverviewWindow(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        handleTableItemDoubleClicked(current);
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleSendMessageWindow(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        int row = current->row();
        QTableWidgetItem* item = this->twTable->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic topic = data.value<Topic>();
            SendMessageWindow* win = new SendMessageWindow;
            connect(this, &TopicsWindow::activateSendMessageWindow, win, &SendMessageWindow::afterWindowActivated);
            emit activateSendMessageWindow(QVariant::fromValue(topic));
            win->exec();
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleDeleteTopic(bool)
{
    QTableWidgetItem* current = this->twTable->currentItem();
    if (current && current->isSelected())
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, tr("Delete Topic"), tr("Are you sure you want to delete this topic?"));
        if (button == QMessageBox::Yes)
        {
            int row = current->row();
            QTableWidgetItem* item = this->twTable->item(row, 0);
            QVariant data = item->data(Qt::UserRole);
            if (data.canConvert<Topic>())
            {
                Topic topic = data.value<Topic>();
                HttpStatusCode error;
                this->m_TopicService->deleteTopic(topic, error);
                if (error.code == HttpStatusCode::StatusCode::NoContent)
                {
                    emit removeTableItem(*item);
                }
                else
                {
                    QMessageBox::critical(this, tr("Error"), error.errorString());
                }
            }
        }
    }
    else
    {
        QMessageBox::warning(this, "Warning", "A row of records must be selected at first.");
    }
}

void TopicsWindow::handleInsertTableItem(const QVariant& _var)
{
    if (_var.canConvert<Topic>())
    {
        Topic topic = _var.value<Topic>();
        if (!existTopic(topic))
        {
            QStringList columns;
            columns << "tenant" << "namesapces" << "name"  << "partitions" << "domain" << "producerNum" << "subscriptionNum";
            int row = this->twTable->rowCount();
            this->twTable->setRowCount(++row);
            TopicData data = topic.toData();
            for (int i = 0, n = columns.length(); i < n; i++)
            {
                QTableWidgetItem* item = new QTableWidgetItem(data[columns[i]]);
                item->setData(Qt::ToolTipRole, QVariant::fromValue(item->text()));
                this->twTable->setItem(row - 1, i, item);
                if (i == 0)
                    item->setData(Qt::UserRole, QVariant::fromValue(topic));
            }
            this->twTable->repaint();
        }
    }
}

void TopicsWindow::handleTableItemPressed(QTableWidgetItem* _item)
{
    if (qApp->mouseButtons() == Qt::RightButton)
    {
        int row = _item->row();
        QTableWidgetItem* first = this->twTable->item(row, 0);
        QVariant data = first->data(Qt::UserRole);
        if (data.canConvert<Topic>())
        {
            Topic topic = data.value<Topic>();
            QMenu* popup = new QMenu(this);
            popup->addAction(this->actOverview);
            popup->addSeparator();
            popup->addAction(this->actDelete);
            popup->addSeparator();
            popup->addAction(this->actLastMessage);
            popup->addAction(this->actQueryData);
            popup->addAction(this->actStorage);
            popup->addAction(this->actSendMessage);
            popup->addSeparator();
            popup->addAction(this->actCopy);
            popup->exec(QCursor::pos());
        }
    }
}

void TopicsWindow::handleTableItemDoubleClicked(QTableWidgetItem* _item)
{
    int row = _item->row();
    QTableWidgetItem* first = this->twTable->item(row, 0);
    QVariant data = first->data(Qt::UserRole);
    if (data.canConvert<Topic>())
    {
        Topic topic = data.value<Topic>();
        TopicOverviewWindow* win = new TopicOverviewWindow;
        connect(this, &TopicsWindow::activateTopicOverviewWindow, win, &TopicOverviewWindow::afterWindowActivated);
        emit activateTopicOverviewWindow(QVariant::fromValue(topic));
        win->exec();
    }
}
