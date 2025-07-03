#ifndef SENDMESSAGEWINDOW_H
#define SENDMESSAGEWINDOW_H

#include <QDialog>

#include "../topic.h"

class QLabel;
class QTextEdit;
class QLineEdit;
class QComboBox;
class ClusterService;

class SendMessageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SendMessageWindow(QWidget* parent = nullptr);

public slots:
    void afterWindowActivated(const QVariant&);

private slots:
    void handleSendMessages();
    void handleCurrentTextChanged(const QString&);

private:
    Topic m_topic;

    QPushButton* btnGet;
    QLineEdit* teServerUrl;
    QLineEdit* teMessageKey;
    QLabel* lblTopicName;
    QTextEdit* teMessage;
    QComboBox* cbCluster;

    ClusterService* m_ClusterService;
};

#endif // SENDMESSAGEWINDOW_H
