#include "sinkinputwindow.h"

#include <QIcon>
#include <QLineEdit>
#include <QFormLayout>
#include <QTableWidget>
#include <QPushButton>
#include <QMessageBox>
#include <QFile>
#include <QSpinBox>
#include <QGuiApplication>

#include "../services/sinkservice.h"

SinkInputWindow::SinkInputWindow(QWidget* parent) : BaseInputWindow(parent), m_SinkService(new SinkService(this)), m_Opt(NEW)
{
    this->tbInputs = new QLineEdit;
    setFixedSize(QSize(680, 580));
    setWindowTitle(tr("Input Sink"));
    setWindowIcon(QIcon(":/export"));
}

void SinkInputWindow::afterWindowActivated(const QVariant& _var)
{
    BaseInputWindow::afterWindowActivated(_var);

    QStringList keys, values;
    keys << "userName" << "password" << "jdbcUrl"  << "tableName" << "nonKey" << "key" << "batchSize" << "schedulePeriod";
    if (_var.canConvert<Namespace>())
    {
        Namespace ns = value<Namespace>();
        this->tbTenant->setText(ns.tenant().name());
        this->tbNamespace->setText(ns.name());
        values << "" << "" << "" << "" << "" << "" << "100" << "300";
        this->twConfigure->setRowCount(keys.length());
        for (int i = 0, n = keys.length(); i < n ; i++)
        {
            QTableWidgetItem* itemKey = new QTableWidgetItem(keys[i]);
            QTableWidgetItem* itemValue = new QTableWidgetItem(values[i]);
            this->twConfigure->setItem(i, 0, itemKey);
            this->twConfigure->setItem(i, 1, itemValue);
        }

        connect(this->btnOk, &QPushButton::clicked, this, &SinkInputWindow::handleNewSink);
    }
    else if (_var.canConvert<Sink>())
    {
        Sink sink = value<Sink>();
        this->tbTenant->setText(sink.getNamespace().tenant().name());
        this->tbNamespace->setText(sink.getNamespace().name());
        this->tbName->setText(sink.name());
        this->tbInputs->setText(sink.inputs().join(","));
        this->spParallelism->setValue(sink.instanceNum());
        int row = 0;
        FunctionConfig config = sink.config();
        this->twConfigure->setRowCount(config.size());
        FunctionConfig::const_iterator it;
        for (it = config.constBegin(); it != config.constEnd(); ++it)
        {
            QTableWidgetItem* itemKey = new QTableWidgetItem(it.key());
            QTableWidgetItem* itemValue = new QTableWidgetItem(it.value());
            this->twConfigure->setItem(row, 0, itemKey);
            this->twConfigure->setItem(row, 1, itemValue);
            row++;
        }

        connect(this->btnOk, &QPushButton::clicked, this, &SinkInputWindow::handleUpdateSink);
    }
}

void SinkInputWindow::additionalWidget(QFormLayout* _layout)
{
    _layout->addRow(tr("&Inputs:"), this->tbInputs);
}

void SinkInputWindow::handleNewSink()
{
    Sink sink;
    if (this->tbName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Name input items must be entered."));
        this->tbName->setFocus();
        return;
    }
    else if (this->tbInputs->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Inputs input items must be entered."));
        this->tbInputs->setFocus();
        return;
    }
    else if (this->tbFile->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Sink package file must be entered."));
        this->tbFile->setFocus();
        return;
    }
    QFile* file = new QFile(this->tbFile->text());
    if (!file->exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("Sink package file is not exist."));
        this->tbFile->setFocus();
        return;
    }
    Namespace ns = value<Namespace>();
    sink.setNamespace(ns);
    sink.setName(this->tbName->text());
    sink.setParallelism(this->spParallelism->value());
    sink.setInstanceNum(sink.parallelism());
    int indexs = this->twConfigure->rowCount();
    sink.setInputs(this->tbInputs->text().split(","));
    for (int i = 0, n = indexs; i < n; i++)
    {
        QTableWidgetItem* itemKey = this->twConfigure->item(i, 0);
        QTableWidgetItem* itemValue = this->twConfigure->item(i, 1);
        if (itemKey != Q_NULLPTR && itemValue != Q_NULLPTR)
        {
            sink.addConfig(itemKey->text(), itemValue->text());
        }
    }
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->btnOk->setEnabled(false);
    HttpStatusCode error;
    this->m_SinkService->create(sink, file, error);
    QGuiApplication::restoreOverrideCursor();
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        this->tbName->clear();
        this->tbInputs->clear();
        this->tbFile->clear();
        QMessageBox::information(this, tr("New Sink"), tr("Create a new Sink successfully."));
        emit completedInput(QVariant::fromValue(sink));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
    this->btnOk->setEnabled(true);
}

void SinkInputWindow::handleUpdateSink()
{
    Sink sink = value<Sink>();
    if (this->tbName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Name input items must be entered."));
        this->tbName->setFocus();
        return;
    }
    else if (this->tbInputs->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Inputs input items must be entered."));
        this->tbInputs->setFocus();
        return;
    }
    QFile* file = 0;
    if (!this->tbFile->text().isEmpty())
    {
        file = new QFile(this->tbFile->text());
        if (!file->exists())
        {
            QMessageBox::critical(this, tr("Error"), tr("Sink package file is not exist."));
            this->tbFile->setFocus();
            return;
        }
    }
    sink.setName(this->tbName->text());
    sink.setParallelism(this->spParallelism->value());
    sink.setInstanceNum(sink.parallelism());
    int indexs = this->twConfigure->rowCount();
    sink.setInputs(this->tbInputs->text().split(","));
    for (int i = 0, n = indexs; i < n; i++)
    {
        QTableWidgetItem* itemKey = this->twConfigure->item(i, 0);
        QTableWidgetItem* itemValue = this->twConfigure->item(i, 1);
        if (itemKey != Q_NULLPTR && itemValue != Q_NULLPTR)
        {
            sink.addConfig(itemKey->text(), itemValue->text());
        }
    }
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->btnOk->setEnabled(false);
    HttpStatusCode error;
    this->m_SinkService->update(sink, file, error);
    QGuiApplication::restoreOverrideCursor();
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        QMessageBox::information(this, tr("Update Sink"), tr("Update a Sink successfully."));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
    this->btnOk->setEnabled(true);
}
