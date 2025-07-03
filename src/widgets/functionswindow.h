#ifndef FUNCTIONSWINDOW_H
#define FUNCTIONSWINDOW_H

#include "basemdisubwindow.h"

class FunctionService;
class QTableWidgetItem;

class FunctionsWindow : public BaseMdiSubWindow
{
    Q_OBJECT

public:
    explicit FunctionsWindow(QWidget* parent = nullptr);

    MdiSubWindow::SubWindowType subWindowType() const override;

signals:
    void activateNewFunctionWindow(const QVariant&);
    void activateUpdateFunctionWindow(const QVariant&);
    void activateFunctionInstanceWindow(const QVariant&);
    //void removeTableItem(const QTableWidgetItem&);
    void completeStartFunction();
    void completeStopFunction();

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    void createActions() override;
    virtual bool doDelete(const QVariant&, HttpStatusCode&) override;
    virtual bool doStart(const QVariant&, HttpStatusCode&) override;
    virtual bool doStop(const QVariant&, HttpStatusCode&) override;

private:
    FunctionService* m_FunctionService;
    QStringList m_Columns;
    QStringList m_Header;

private slots:
    void handleNewFunction(bool);
    void handleUpdateFunction(bool);
    void handleReload();
    void handleInsertNewFunction(const QVariant&);
    void handleFunctionInstances(bool);
    void handleTableItemDoubleClicked(QTableWidgetItem*);

};

#endif // FUNCTIONSWINDOW_H
