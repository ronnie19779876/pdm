#ifndef PEEKMESSAGESWINDOW_H
#define PEEKMESSAGESWINDOW_H

#include <QDialog>

#include "../topic.h"
#include "../subscription.h"

class QLabel;
class QTableWidget;
class QTextEdit;
class QComboBox;
class QSpinBox;
class TopicService;
class CursorService;

class PeekMessagesWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PeekMessagesWindow(QWidget* parent = nullptr);
    virtual ~PeekMessagesWindow();

signals:
    void initialize();

public slots:
    void afterWindowActivated(const Topic&, const Subscription&, const int&);

private:
    Topic m_Topic;
    Subscription m_Subscription;
    int m_Partitions;

    TopicService* m_TopicService;
    CursorService* m_CursorService;

    QLabel* lblTopicName;
    QLabel* lblBacklog;
    QTableWidget* twMessages;
    QTextEdit* teProperties;
    QTextEdit* teKey;
    QTextEdit* teMessage;
    QComboBox* cbSchema;
    QSpinBox* sbNumber;
    QPushButton* btnPeek;

private slots:
    void handleInitialize();
    void handleValueChanged(int);
    void handleItemSelectionChanged();
    void handleCurrentTextChanged(const QString&);

};

#endif // PEEKMESSAGESWINDOW_H
