#ifndef NEWTOPICWINDOW_H
#define NEWTOPICWINDOW_H

#include <QDialog>

#include "../namespace.h"

class QButtonGroup;
class QRadioButton;
class QLineEdit;
class TopicService;
class QSpinBox;

class NewTopicWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NewTopicWindow(QWidget* parent = nullptr);

signals:
    void completedNewTopic(const QVariant&);

public slots:
    void afterWindowActivated(const QVariant&);

private:
    QRadioButton* rbPersistent;
    QRadioButton* rbNonPersistent;
    QLineEdit* tbTopicName;
    QSpinBox* sbPartitions;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    Namespace m_Namespace;
    TopicService* m_TopicService;

private slots:
    void handleNewTopic(bool);

};

#endif // NEWTOPICWINDOW_H
