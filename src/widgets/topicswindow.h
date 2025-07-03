#ifndef TOPICSWINDOW_H
#define TOPICSWINDOW_H

#include "basemdisubwindow.h"

class QTableWidgetItem;
class TopicService;
class Topic;

class TopicsWindow : public BaseMdiSubWindow
{
    Q_OBJECT

public:
    explicit TopicsWindow(QWidget* parent = Q_NULLPTR);

    MdiSubWindow::SubWindowType subWindowType() const override;

public slots:
    void afterWindowActivated(const QVariant&) override;

signals:
    void insertTableItem();
    void activateLastCommitMessageWindow(const QVariant&);
    void activateNewTopicWindow(const QVariant&);
    void activateTopicStorageWindow(const QVariant&);
    void activateQueryTopicDataWindow(const QVariant&);
    void activateTopicOverviewWindow(const QVariant&);
    void activateSendMessageWindow(const QVariant&);

private:
    bool existTopic(const Topic&);

private:
    TopicService* m_TopicService;
    QStringList m_Columns;

    QAction* actLastMessage;
    QAction* actQueryData;
    QAction* actCopy;
    QAction* actStorage;
    QAction* actOverview;
    QAction* actSendMessage;

private slots:
    void handleReload();
    void handleNewTopic(bool);
    void handleDeleteTopic(bool);
    void handleInsertTableItem(const QVariant&);
    void handleTableItemPressed(QTableWidgetItem*);
    void handleTableItemDoubleClicked(QTableWidgetItem*);
    void handleLastCommitMessageWindow(bool);
    void handleTopicStorageWindow(bool);
    void handleQueryTopicDataWindow(bool);
    void handleTopicOverviewWindow(bool);
    void handleSendMessageWindow(bool);

};

#endif // TOPICSWINDOW_H
