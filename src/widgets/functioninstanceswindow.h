#ifndef FUNCTIONINSTANCESWINDOW_H
#define FUNCTIONINSTANCESWINDOW_H

#include <QDialog>
#include <QVariant>

class QTableWidget;
class SourceService;
class SinkService;
class Sink;
class Source;
class Function;
class FunctionService;

class FunctionInstancesWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FunctionInstancesWindow(QWidget* parent = nullptr);

signals:
    void initialize(const QVariant&);

public slots:
    void afterWindowActivated(const QVariant&);

private:
    SourceService* m_SourceService;
    SinkService* m_SinkService;
    FunctionService* m_FunctionService;

    QVariant m_Variant;
    QTableWidget* twInstances;
    QStringList m_Columns;

private slots:
    void handleInitialize(const QVariant&);
    void handleLoadSinkInstances(const Sink&);
    void handleLoadSourceInstances(const Source&);
    void handleLoadFunctionInstances(const Function&);
};

#endif // FUNCTIONINSTANCESWINDOW_H
