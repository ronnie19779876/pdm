#ifndef SOURCEINPUTWINDOW_H
#define SOURCEINPUTWINDOW_H

#include "baseinputwindow.h"

class SourceService;

class SourceInputWindow : public BaseInputWindow
{
    Q_OBJECT

public:
    explicit SourceInputWindow(QWidget* parent = nullptr);

signals:

public slots:
    virtual void afterWindowActivated(const QVariant&) override;

protected:
    virtual void additionalWidget(QFormLayout*) override;

private:
    QLineEdit* tbClassname;
    QLineEdit* tbTopicName;

private:
    SourceService* m_SourceService;

private slots:
    void handleNewSource(bool);
    void handleUpdateSource(bool);

};

#endif // SOURCEINPUTWINDOW_H
