#ifndef BASEMDISUBWINDOW_H
#define BASEMDISUBWINDOW_H

#include "mdisubwindow.h"

#include <QVariant>

#include "../services/httpclient.h"

class QTableWidget;
class QToolBar;
class QTableWidgetItem;

class BaseMdiSubWindow : public MdiSubWindow
{
    Q_OBJECT

public:
    explicit BaseMdiSubWindow(QWidget* parent = nullptr);

    MdiSubWindow::SubWindowType subWindowType() const override;

    inline QVariant variant() const { return this->m_Variant; }
    inline void setVariant(const QVariant& _var) { this->m_Variant = _var; }

signals:
    void initialize();
    void removeTableItem(const QTableWidgetItem&);
    void completeStart();
    void completeStop();

public slots:
    virtual void afterWindowActivated(const QVariant&);

protected:
    virtual void createActions();
    virtual void createToolbar();
    virtual bool doDelete(const QVariant&, HttpStatusCode&);
    virtual bool doStart(const QVariant&, HttpStatusCode&);
    virtual bool doStop(const QVariant&, HttpStatusCode&);

    template<typename T>
    inline T value() const { return this->m_Variant.value<T>(); };

protected:
    QTableWidget* twTable;
    QMenu* meuPopupMenu;
    QToolBar* tbToolbar;

    bool m_HasCreateActions;

protected:
    QAction* actNew;
    QAction* actUpdate;
    QAction* actStart;
    QAction* actStop;
    QAction* actDelete;
    QAction* actRefresh;
    QAction* actClose;
    QAction* actCopyCellText;
    QAction* actCopyRowText;
    QAction* actInstances;

protected slots:
    void handleRemoveTableItem(const QTableWidgetItem&);
    void handleCopyCellText(bool);

private slots:
    void handleTableContextMenu(const QPoint& pos);
    void handleCopyRowText(bool);
    void handleDelete(bool);
    void handleStart(bool);
    void handleStop(bool);

private:
    QVariant m_Variant;

};

#endif // BASEMDISUBWINDOW_H
