#ifndef LASTCOMMITMESSAGEWINDOW_H
#define LASTCOMMITMESSAGEWINDOW_H

#include <QDialog>

#include "../topic.h"

class QComboBox;
class QTextEdit;
class QLabel;
class QSpinBox;
class QTableWidget;
class TopicService;

class LastCommitMessageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit LastCommitMessageWindow(QWidget* parent = nullptr);

signals:

public slots:
    void afterWindowActivated(const QVariant&);

private:
    TopicService* m_TopicService;
    Topic m_topic;

    QComboBox* cbPartitions;
    QPushButton* btnGet;
    QLabel* lblMessageId;
    QSpinBox* sbNumber;
    QLabel* lblTopicName;
    QTableWidget* twMessages;
    QTextEdit* teProperties;
    QTextEdit* teKey;
    QTextEdit* teMessage;
    QComboBox* cbSchema;

private slots:
    void handleGetMessages();
    void handleCurrentIndexChanged(const QString&);
    void handleItemSelectionChanged();
    void handleCurrentTextChanged(const QString&);
};

#endif // LASTCOMMITMESSAGEWINDOW_H
