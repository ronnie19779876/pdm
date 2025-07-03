#include "newfunctionwindow.h"

#include <QFormLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSpinBox>
#include <QFile>
#include <QGuiApplication>

#include "../services/functionservice.h"

NewFunctionWindow::NewFunctionWindow(QWidget* _parent) : BaseInputWindow(_parent), m_FunctionService(new FunctionService(this))
{
    this->tbClassname = new QLineEdit;
    this->tbInputs = new QLineEdit;
    this->tbOutput = new QLineEdit;

    setWindowIcon(QIcon(":/function"));
}

void NewFunctionWindow::afterWindowActivated(const QVariant& _var)
{
    BaseInputWindow::afterWindowActivated(_var);
    if (_var.canConvert<Namespace>())
    {
        Namespace ns = value<Namespace>();
        this->tbTenant->setText(ns.tenant().name());
        this->tbNamespace->setText(ns.name());
        setWindowTitle(tr("New Function"));
        connect(this->btnOk, &QPushButton::clicked, this, &NewFunctionWindow::handleNewFunction);
    }
    else if (_var.canConvert<Function>())
    {
        Function function = value<Function>();
        this->tbTenant->setText(function.getNamespace().tenant().name());
        this->tbNamespace->setText(function.getNamespace().name());
        this->tbName->setText(function.name());
        this->tbName->setReadOnly(true);
        this->tbClassname->setText(function.classname());
        this->tbInputs->setText(function.inputs());
        this->tbOutput->setText(function.output());
        this->spParallelism->setValue(function.instanceNum());
        setWindowTitle(tr("Update Function"));
        connect(this->btnOk, &QPushButton::clicked, this, &NewFunctionWindow::handleUpdateFunction);
    }
}

void NewFunctionWindow::additionalWidget(QFormLayout* _layout)
{
    _layout->addRow(tr("&ClassName:"), this->tbClassname);
    _layout->addRow(tr("&Inputs:"), this->tbInputs);
    _layout->addRow(tr("&Output:"), this->tbOutput);
}

bool NewFunctionWindow::needConfigurationTable()
{
    return false;
}

void NewFunctionWindow::handleNewFunction(bool)
{
    //检查输入项
    if (this->tbName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Name input items must be entered."));
        this->tbName->setFocus();
        return;
    }
    else if (this->tbClassname->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("ClassName input items must be entered."));
        this->tbClassname->setFocus();
        return;
    }
    else if (this->tbInputs->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Inputs input items must be entered."));
        this->tbInputs->setFocus();
        return;
    }
    else if (this->tbOutput->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Output input items must be entered."));
        this->tbOutput->setFocus();
        return;
    }
    else if (this->tbFile->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Function config file must be entered."));
        this->tbFile->setFocus();
        return;
    }
    Namespace ns = value<Namespace>();
    Function function;
    function.setNamespace(ns);
    function.setName(this->tbName->text());
    function.setClassname(this->tbClassname->text());
    function.setInputs(this->tbInputs->text());
    function.setOutput(this->tbOutput->text());
    function.setParallelism(this->spParallelism->value());
    function.setInstanceNum(function.parallelism());
    HttpStatusCode error;
    QFile* file = new QFile(this->tbFile->text());
    if (file->exists())
    {
        QGuiApplication::setOverrideCursor(Qt::WaitCursor);
        this->btnOk->setEnabled(false);
        this->m_FunctionService->createFunction(function, file, error);
        QGuiApplication::restoreOverrideCursor();
        if (error.code == HttpStatusCode::StatusCode::NoContent)
        {
            this->tbName->clear();
            this->tbClassname->clear();
            this->tbInputs->clear();
            this->tbOutput->clear();
            this->tbFile->clear();
            function.appendStatus(this->m_FunctionService->status(function));
            QMessageBox::information(this, tr("New Function"), tr("Create a new Function successfully."));
            emit completedInput(QVariant::fromValue(function));
            close();
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), error.errorString());
        }
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), tr("Function package file is not exist."));
    }
    this->btnOk->setEnabled(true);
}

void NewFunctionWindow::handleUpdateFunction(bool)
{
    if (this->tbClassname->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("ClassName input items must be entered."));
        this->tbClassname->setFocus();
        return;
    }
    else if (this->tbInputs->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Inputs input items must be entered."));
        this->tbInputs->setFocus();
        return;
    }
    else if (this->tbOutput->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Output input items must be entered."));
        this->tbOutput->setFocus();
        return;
    }
    QFile* file = 0;
    if (!this->tbFile->text().isEmpty())
    {
        file = new QFile(this->tbFile->text());
        if (!file->exists())
        {
            QMessageBox::critical(this, tr("Error"), tr("Function package file is not exist."));
            this->tbFile->setFocus();
            return;
        }
    }
    Function function = value<Function>();
    function.setName(this->tbName->text());
    function.setClassname(this->tbClassname->text());
    function.setInputs(this->tbInputs->text());
    function.setOutput(this->tbOutput->text());
    function.setParallelism(this->spParallelism->value());
    function.setInstanceNum(function.parallelism());
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->btnOk->setEnabled(false);
    HttpStatusCode error;
    this->m_FunctionService->updateFunction(function, file, error);
    QGuiApplication::restoreOverrideCursor();
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        QMessageBox::information(this, tr("Update Function"), tr("Update a Function successfully."));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
    this->btnOk->setEnabled(true);
}
