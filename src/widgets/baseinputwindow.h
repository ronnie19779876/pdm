#ifndef BASEINPUTWINDOW_H
#define BASEINPUTWINDOW_H

#include <QDialog>
#include <QVariant>

class QLineEdit;
class QSpinBox;
class QTableWidget;
class QMenu;
class QFormLayout;

class BaseInputWindow : public QDialog
{
    Q_OBJECT

public:
    explicit BaseInputWindow(QWidget* parent = nullptr);

signals:
    void completedInput(const QVariant&);

public slots:
    virtual void afterWindowActivated(const QVariant&);

protected:
    virtual void additionalWidget(QFormLayout*);
    virtual bool needConfigurationTable();

    void createActions();

    template<typename T>
    inline T value() const { return this->m_Variant.value<T>(); };

protected:
    QLineEdit* tbTenant;
    QLineEdit* tbNamespace;
    QLineEdit* tbName;
    QSpinBox* spParallelism;
    QLineEdit* tbFile;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QPushButton* btnBrowse;
    QTableWidget* twConfigure;
    QMenu* meuPopupMenu;

private:
    QAction* actAddRow;
    QAction* actDeleteRow;
    QAction* actCopyCellText;

private:
    QVariant m_Variant;

private slots:
    void handleBrowseFiles();
    void handleTableContextMenu(const QPoint&);
    void handleAddRow(bool);
    void handleDeleteRow(bool);
    void handleCopyCellText(bool);
};

#endif // BASEINPUTWINDOW_H
