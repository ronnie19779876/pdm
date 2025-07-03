#ifndef NEWCLUSTERWINDOW_H
#define NEWCLUSTERWINDOW_H

#include <QDialog>
#include <QLineEdit>

class Cluster;

class NewClusterWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NewClusterWindow(QWidget* parent = nullptr);

signals:
    void completeNewCluster(const Cluster& _cluster);
    void completeUpdateCluster(const Cluster& _cluster);

public slots:
    void afterWindowActivated(const Cluster* _cluster = nullptr);

private:
    QLineEdit* tbName;
    QLineEdit* tbServiceUrl;
    QLineEdit* tbFunctionUrl;
    QLineEdit* tbPrestoUrl;
    QLineEdit* tbAuthToken;
    QPushButton* btnOk;
    QPushButton* btnCancel;

    bool m_New;

private slots:
    void handleNewCluster();

};

#endif // NEWCLUSTERWINDOW_H
