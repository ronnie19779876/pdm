#ifndef SOURCESWINDOW_H
#define SOURCESWINDOW_H

#include "basemdisubwindow.h"

class Namespace;
class SourceService;
class Source;

class SourcesWindow : public BaseMdiSubWindow
{
    Q_OBJECT

public:
    explicit SourcesWindow(QWidget* parent = nullptr);

    MdiSubWindow::SubWindowType subWindowType() const override;

signals:
    void activateNewSourceWindow(const QVariant&);
    void activateUpdateSourceWindow(const QVariant&);
    void activateSourceInstanceWindow(const QVariant&);

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    void createActions() override;
    virtual bool doDelete(const QVariant&, HttpStatusCode&) override;
    virtual bool doStart(const QVariant&, HttpStatusCode&) override;
    virtual bool doStop(const QVariant&, HttpStatusCode&) override;

private slots:
    void handleReload();
    void handleNewSource(bool);
    void handleUpdateSource(bool);
    void handleSourceInstances();
    void handleInsertNewSource(const QVariant&);
    void handleTableItemDoubleClicked(QTableWidgetItem*);

private:
    SourceService* m_SourceService;

};

#endif // SOURCESWINDOW_H
