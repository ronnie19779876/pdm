#ifndef QUERYTOPICDATAWINDOW_H
#define QUERYTOPICDATAWINDOW_H

#include <QDialog>
#include <QVariant>

class QLabel;
class QTextEdit;
class QTableWidget;
class PrestoQueryService;
class Statement;
class QMenu;

class QueryTopicDataWindow : public QDialog
{
    Q_OBJECT

public:
    explicit QueryTopicDataWindow(QWidget* parent = nullptr);
    virtual ~QueryTopicDataWindow();

signals:
    void queryNext();
    void running();
    void finish();
    void error();

public slots:
    void afterWindowActivated(const QVariant&);

private:
    QPushButton* btnQuery;
    QPushButton* btnStop;
    QLabel* lblTopicName;
    QLabel* lblStatus;
    QTextEdit* teCondition;
    QTableWidget* twResult;
    QAction* actCopyCellText;
    QAction* actCopyRowText;
    QMenu* meuPopupMenu;
    QVariant m_Variant;

    PrestoQueryService* m_Query;
    Statement* m_Statement;

private slots:
    void handleQuery();
    void handleQueryNext();
    void handleRunning();
    void handleFinish();
    void handleError();
    void handleCancelQuery();
    void handleTableContextMenu(const QPoint& pos);
    void handleCopyRowText(bool);
    void handleCopyCellText(bool);
};

#endif // QUERYTOPICDATAWINDOW_H
