#ifndef TOPICOVERVIEWWINDOW_H
#define TOPICOVERVIEWWINDOW_H

#include <QDialog>
#include <QVariant>

class TopicService;
class QTableWidget;
class QComboBox;
class QLabel;
class Topic;
class QTableWidgetItem;
class Subscription;
class QMenu;
class Cursors;

class TopicOverviewWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TopicOverviewWindow(QWidget* parent = nullptr);

signals:
    void initialize();
    void load(const Topic&, const int&);
    void load(const Subscription&);
    void activatePeekNthMessagesWindow(const Topic&, const Subscription&, const int&);
    void activateNewSubscriptionWindow(const QVariant&);

public slots:
    void afterWindowActivated(const QVariant&);

private:
    QVariant m_Variant;
    TopicService* m_TopicService;

    QComboBox* cbPartitions;
    QLabel* lblTopicName;
    QPushButton* btnNewSubsription;
    QTableWidget* twProducers;
    QTableWidget* twSubscriptions;
    QTableWidget* twConsumers;
    QTableWidget* twCurrent;
    QMenu* meuProducers;
    QMenu* meuConsumers;

    QAction* actCopyCellText;
    QAction* actCopyRowText;
    QAction* actPeekMsg;
    QAction* actRemoveSubscription;

private:
    void createActions();

private slots:
    void handleInitialize();
    void handleCurrentIndexChanged(const QString&);
    void handleLoad(const Topic&, const int&);
    void handleLoad(const Subscription&);
    void handleItemDoubleClicked(QTableWidgetItem*);
    void handleProducerTableContextMenu(const QPoint& pos);
    void handleConsumerTableContextMenu(const QPoint& pos);
    void handleCopyCellText(bool);
    void handleCopyRowText(bool);
    void handleActivatePeekMessagesWindow(bool);
    void handleActivateNewSubscriptionWindow(bool);
    void handleDeleteSubscription(bool);

};

#endif // TOPICOVERVIEWWINDOW_H
