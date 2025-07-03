#ifndef SUBSCRIPTIONWINDOW_H
#define SUBSCRIPTIONWINDOW_H

#include <QDialog>
#include <QVariant>

class Subscription;
class QLineEdit;
class TopicService;

class SubscriptionWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SubscriptionWindow(QWidget* parent = nullptr);

signals:
    void completedNewSubscription();

public slots:
    void afterWindowActivated(const QVariant&);

private:
    QLineEdit* tbName;

    QPushButton* btnOk;
    QPushButton* btnCancel;

    TopicService* m_TopicService;
    QVariant m_Var;

private slots:
    void handleNewSubscription();

};

#endif // SUBSCRIPTIONWINDOW_H
