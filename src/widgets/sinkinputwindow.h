#ifndef SINKINPUTWINDOW_H
#define SINKINPUTWINDOW_H

#include "baseinputwindow.h"

class Sink;
class QLineEdit;
class QSpinBox;
class QTableWidget;
class QMenu;
class SinkService;

class SinkInputWindow : public BaseInputWindow
{
    Q_OBJECT

public:
    enum Operation
    {
        NEW = 0,
        UPDATE,
    };

    explicit SinkInputWindow(QWidget* parent = nullptr);

signals:

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    virtual void additionalWidget(QFormLayout*) override;

private:
    QLineEdit* tbInputs;

private:
    SinkService* m_SinkService;
    Operation m_Opt;

private slots:
    void handleNewSink();
    void handleUpdateSink();

};

#endif // SINKINPUTWINDOW_H
