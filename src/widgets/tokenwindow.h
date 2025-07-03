#ifndef TOKENWINDOW_H
#define TOKENWINDOW_H

#include <QDialog>

class QPushButton;
class QLineEdit;
class QTableWidget;
class QTableWidgetItem;
class TokenService;
class QMenu;
class Token;

class TokenWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TokenWindow(QWidget* parent = nullptr);
    virtual ~TokenWindow();
    void initialize();

signals:
    void completedAddnewToken(const int&, const Token&);
    void removeTableItem(const QTableWidgetItem&);

private:
    QLineEdit* tbSubject;
    QLineEdit* tbKey;
    QPushButton* btnOk;
    QPushButton* btnCancel;
    QPushButton* btnSelectFile;
    QPushButton* btnGenerateKey;
    QTableWidget* twTable;
    QMenu* meuPopupMenu;
    QAction* actCopyCellText;
    QAction* actDelete;
    QStringList m_Headers;

    TokenService* m_TokenService;

private slots:
    void handleNewToken(bool);
    void handleSelectedFile(bool);
    void handleGenerateSecretKey(bool);
    void handleTableContextMenu(const QPoint& pos);
    void handleCopyCellText(bool);
    void handleInsertTableItem(const int&, const Token&);
    void handleDeleteToken(bool);
    void handleRemoveTableItem(const QTableWidgetItem&);
};

#endif // TOKENWINDOW_H
