#include "sendmessagewindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QTextEdit>
#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QComboBox>
#include <QDebug>

#include <pulsar/Client.h>

#include "../services/clusterservice.h"

using namespace pulsar;

SendMessageWindow::SendMessageWindow(QWidget* _parent): QDialog(_parent), m_ClusterService(new ClusterService(this))
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblTopicName = new QLabel;
    this->teServerUrl = new QLineEdit();
    this->teMessageKey = new QLineEdit();
    this->teMessage = new QTextEdit();
    this->cbCluster = new QComboBox;
    formLayout->addRow(tr("Allowed &Clusters:"), cbCluster);
    formLayout->addRow(tr("&Broker Service URL:"), teServerUrl);
    formLayout->addRow(tr("&Topic Name:"), lblTopicName);
    formLayout->addRow(tr("Message &Key:"), teMessageKey);
    formLayout->addRow(tr("&Message Body:"), teMessage);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnGet = new QPushButton(tr("&Send"));
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnGet = new QPushButton(okIcon, tr("&Send"));
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(this->btnGet);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(btnCancel);

    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedWidth(920);
    setFixedHeight(630);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Send Message"));
    setWindowIcon(QIcon(":/images/message.ico"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnGet, &QPushButton::clicked, this, &SendMessageWindow::handleSendMessages);
    connect(btnCancel, &QPushButton::clicked, this, &SendMessageWindow::close);
    connect(this->cbCluster, &QComboBox::currentTextChanged, this, &SendMessageWindow::handleCurrentTextChanged);
}

void SendMessageWindow::afterWindowActivated(const QVariant& _var)
{
    if (_var.canConvert<Topic>())
    {
        this->m_topic = _var.value<Topic>();
        if (!this->m_topic.authToken().isEmpty())
        {
            this->m_ClusterService->setAuthToken(this->m_topic.authToken());
        }
        this->lblTopicName->setText(QString("%1://%2/%3/%4").arg(m_topic.domain(), m_topic.getNamespace().tenant().name(), m_topic.getNamespace().name(), m_topic.name()));
        Cluster cluster = this->m_topic.getNamespace().tenant().cluster();
        this->cbCluster->addItems(this->m_ClusterService->clusters(cluster));
    }
}

void SendMessageWindow::handleSendMessages()
{
    if (this->teServerUrl->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Pulsar Server URL is required."));
        this->teServerUrl->setFocus();
        return;
    }
    if (this->teMessage->toPlainText().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Pulsar Message body is required."));
        this->teMessage->setFocus();
        return;
    }
    Client client(this->teServerUrl->text().toStdString());
    pulsar::Producer producer;
    Result result = client.createProducer(this->lblTopicName->text().toStdString(), producer);
    if (result != ResultOk)
    {
        QMessageBox::warning(this, "Warning", "Error creating producer.");
    }
    else
    {
        qDebug() << "successfully creating producer: " << this->lblTopicName->text() << Qt::endl;
        MessageBuilder builder;
        if (!this->teMessageKey->text().isEmpty())
        {
            builder.setPartitionKey(this->teMessageKey->text().toStdString());
        }
        Message message = builder.setContent(this->teMessage->toPlainText().toStdString()).build();
        Result result = producer.send(message);
        if (result != ResultOk)
        {
            QMessageBox::critical(this, "Error", "The message could not be sent.");
        }
        else
        {
            QMessageBox::information(this, "Success", "The message sent successfully.");
        }

    }
    client.close();
}

void SendMessageWindow::handleCurrentTextChanged(const QString& _text)
{
    qDebug() << "current cluster name: " << _text << Qt::endl;
    QString broker(this->m_ClusterService->brokerServiceUrl(this->m_topic.getNamespace().tenant().cluster(), _text));
    this->teServerUrl->setText(broker);
}
