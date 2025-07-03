#ifndef SINKSWINDOW_H
#define SINKSWINDOW_H

#include "basemdisubwindow.h"

class QTableWidgetItem;
class QTableWidget;
class SinkService;
class Sink;

class SinksWindow : public BaseMdiSubWindow
{
    Q_OBJECT

public:
    explicit SinksWindow(QWidget* parent = nullptr);
    virtual ~SinksWindow();

    MdiSubWindow::SubWindowType subWindowType() const override;

signals:
    void activateNewSinkWindow(const QVariant&);
    void activateUpdateSinkWindow(const QVariant&);
    void activateSinkInstanceWindow(const QVariant&);
    void removeTableItem(const QTableWidgetItem&);
    void completeStartSink();
    void completeStopSink();

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    void createActions() override;
    virtual bool doDelete(const QVariant&, HttpStatusCode&) override;
    virtual bool doStart(const QVariant&, HttpStatusCode&) override;
    virtual bool doStop(const QVariant&, HttpStatusCode&) override;

private slots:
    void handleNewSink(bool);
    void handleUpdateSink(bool);
    void handleReload();
    void handleInsertNewSink(const QVariant&);
    void handleSinkInstances(bool);
    void handleTableItemDoubleClicked(QTableWidgetItem*);

private:
    SinkService* m_SinkService;
    QStringList m_Columns;

};

#endif // SINKSWINDOW_H
