#include "sourceinputwindow.h"

#include <QLineEdit>
#include <QIcon>
#include <QTableWidget>
#include <QPushButton>
#include <QSpinBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QFile>
#include <QGuiApplication>

#include "../services/sourceservice.h"

SourceInputWindow::SourceInputWindow(QWidget* parent) : BaseInputWindow(parent), m_SourceService(new SourceService(this))
{
    this->tbClassname = new QLineEdit;
    this->tbTopicName = new QLineEdit;
    setFixedSize(QSize(680, 480));
    setWindowTitle(tr("New Source"));
    setWindowIcon(QIcon(":/images/sources.ico"));
}

void SourceInputWindow::afterWindowActivated(const QVariant& _var)
{
    BaseInputWindow::afterWindowActivated(_var);

    QStringList keys, values;
    keys << "bootstrapServers" << "groupId" << "topic"  << "autoCommitEnabled";
    values << "" << "" << "" << "true";
    if (_var.canConvert<Namespace>())
    {
        Namespace ns = value<Namespace>();
        this->tbTenant->setText(ns.tenant().name());
        this->tbNamespace->setText(ns.name());
        this->twConfigure->setRowCount(keys.length());
        for (int i = 0, n = keys.length(); i < n ; i++)
        {
            QTableWidgetItem* itemKey = new QTableWidgetItem(keys[i]);
            QTableWidgetItem* itemValue = new QTableWidgetItem(values[i]);
            this->twConfigure->setItem(i, 0, itemKey);
            this->twConfigure->setItem(i, 1, itemValue);
        }

        connect(this->btnOk, &QPushButton::clicked, this, &SourceInputWindow::handleNewSource);
    }
    else if (_var.canConvert<Source>())
    {
        setWindowTitle(tr("Update Source"));
        Source source = value<Source>();
        this->tbTenant->setText(source.getNamespace().tenant().name());
        this->tbNamespace->setText(source.getNamespace().name());
        this->tbName->setText(source.name());
        this->tbClassname->setText(source.classname());
        this->tbTopicName->setText(source.topicName());
        this->spParallelism->setValue(source.instanceNum());

        int row = 0;
        FunctionConfig config = source.config();
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

        connect(this->btnOk, &QPushButton::clicked, this, &SourceInputWindow::handleUpdateSource);
    }
}

void SourceInputWindow::additionalWidget(QFormLayout* _layout)
{
    _layout->addRow(tr("&ClassName:"), this->tbClassname);
    _layout->addRow(tr("T&opicName:"), this->tbTopicName);
}

void SourceInputWindow::handleNewSource(bool)
{
    Source source;
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
    else if (this->tbTopicName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("TopicName input items must be entered."));
        this->tbTopicName->setFocus();
        return;
    }
    else if (this->tbFile->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Source package file must be entered."));
        this->tbFile->setFocus();
        return;
    }
    QFile* file = new QFile(this->tbFile->text());
    if (!file->exists())
    {
        QMessageBox::critical(this, tr("Error"), tr("Source package file is not exist."));
        this->tbFile->setFocus();
        return;
    }
    Namespace ns = value<Namespace>();
    source.setNamespace(ns);
    source.setName(this->tbName->text());
    source.setClassname(this->tbClassname->text());
    source.setTopicName(this->tbTopicName->text());
    source.setParallelism(this->spParallelism->value());
    source.setInstanceNum(source.parallelism());
    int indexs = this->twConfigure->rowCount();
    for (int i = 0, n = indexs; i < n; i++)
    {
        QTableWidgetItem* itemKey = this->twConfigure->item(i, 0);
        QTableWidgetItem* itemValue = this->twConfigure->item(i, 1);
        if (itemKey != Q_NULLPTR && itemValue != Q_NULLPTR)
        {
            source.addConfig(itemKey->text(), itemValue->text());
        }
    }
    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->btnOk->setEnabled(false);
    HttpStatusCode error;
    this->m_SourceService->createSource(source, file, error);
    QGuiApplication::restoreOverrideCursor();
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        this->tbName->clear();
        this->tbClassname->clear();
        this->tbTopicName->clear();
        this->tbFile->clear();
        QMessageBox::information(this, tr("New Source"), tr("Create a new Source successfully."));
        emit completedInput(QVariant::fromValue(source));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
    this->btnOk->setEnabled(true);
}

void SourceInputWindow::handleUpdateSource(bool)
{
    Source source = value<Source>();
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
    else if (this->tbTopicName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("TopicName input items must be entered."));
        this->tbTopicName->setFocus();
        return;
    }

    QFile* file = 0;
    if (!this->tbFile->text().isEmpty())
    {
        file = new QFile(this->tbFile->text());
        if (!file->exists())
        {
            QMessageBox::critical(this, tr("Error"), tr("Source package file is not exist."));
            this->tbFile->setFocus();
            return;
        }
    }

    source.setName(this->tbName->text());
    source.setClassname(this->tbClassname->text());
    source.setTopicName(this->tbTopicName->text());
    source.setParallelism(this->spParallelism->value());
    source.setInstanceNum(source.parallelism());
    int indexs = this->twConfigure->rowCount();
    for (int i = 0, n = indexs; i < n; i++)
    {
        QTableWidgetItem* itemKey = this->twConfigure->item(i, 0);
        QTableWidgetItem* itemValue = this->twConfigure->item(i, 1);
        if (itemKey != Q_NULLPTR && itemValue != Q_NULLPTR)
        {
            source.addConfig(itemKey->text(), itemValue->text());
        }
    }

    QGuiApplication::setOverrideCursor(Qt::WaitCursor);
    this->btnOk->setEnabled(false);
    HttpStatusCode error;
    this->m_SourceService->updateSource(source, file, error);
    QGuiApplication::restoreOverrideCursor();
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        QMessageBox::information(this, tr("Update Source"), tr("Update a Source successfully."));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
    this->btnOk->setEnabled(true);
}
