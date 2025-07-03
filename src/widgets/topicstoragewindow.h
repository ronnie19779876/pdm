#ifndef TOPICSTORAGEWINDOW_H
#define TOPICSTORAGEWINDOW_H

#include <QDialog>
#include <QVariant>

class TopicService;
class QComboBox;
class QLabel;
class QTableWidget;
class TopicStorage;

class TopicStorageWindow : public QDialog
{
    Q_OBJECT
public:
    explicit TopicStorageWindow(QWidget* parent = nullptr);

signals:
    void initialize();
    void loadTable(const TopicStorage&);

public slots:
    void afterWindowActivated(const QVariant&);

private:
    QVariant m_Variant;
    TopicService* m_TopicService;

private:
    QComboBox* cbPartitions;
    QLabel* lblSegments;
    QLabel* lblStorageSize;
    QLabel* lblEntries;
    QTableWidget* twSegments;

private slots:
    void handleInitialize();
    void handleCurrentIndexChanged(const QString&);
    void handleLoadTable(const TopicStorage&);

};

#endif // TOPICSTORAGEWINDOW_H
