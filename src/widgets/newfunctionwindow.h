#ifndef NEWFUNCTIONWINDOW_H
#define NEWFUNCTIONWINDOW_H

#include "baseinputwindow.h"


class FunctionService;
class Function;
class QLineEdit;
class QPushButton;

class NewFunctionWindow : public BaseInputWindow
{
    Q_OBJECT

public:
    explicit NewFunctionWindow(QWidget* parent = nullptr);

signals:

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    virtual void additionalWidget(QFormLayout*) override;
    virtual bool needConfigurationTable() override;

private:
    QLineEdit* tbClassname;
    QLineEdit* tbInputs;
    QLineEdit* tbOutput;

    FunctionService* m_FunctionService;

private slots:
    void handleNewFunction(bool);
    void handleUpdateFunction(bool);

};

#endif // NEWFUNCTIONWINDOW_H
