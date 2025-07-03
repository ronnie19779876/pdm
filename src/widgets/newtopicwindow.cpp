#include "newtopicwindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QButtonGroup>
#include <QRadioButton>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QSpinBox>

#include "../services/topicservice.h"

NewTopicWindow::NewTopicWindow(QWidget* parent) : QDialog(parent), m_TopicService(new TopicService(this))
{
    QVBoxLayout* layout = new QVBoxLayout;

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->setContentsMargins(0, 0, 0, 0);
    QButtonGroup* bgDomainGroup = new QButtonGroup(this);
    this->rbPersistent = new QRadioButton(tr("Persistent"));
    this->rbPersistent->setChecked(true);
    this->rbNonPersistent = new QRadioButton(tr("Non-persistent"));
    bgDomainGroup->addButton(this->rbPersistent);
    bgDomainGroup->addButton(this->rbNonPersistent);
    buttonsLayout->addWidget(this->rbPersistent);
    buttonsLayout->addWidget(this->rbNonPersistent);

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    QWidget* group = new QWidget;
    group->setContentsMargins(0, 0, 0, 0);
    group->setLayout(buttonsLayout);
    formLayout->addRow(tr("&Domain:"), group);
    this->tbTopicName = new QLineEdit;
    formLayout->addRow(tr("&Topic Name:"), this->tbTopicName);
    this->sbPartitions = new QSpinBox;
    this->sbPartitions->setRange(0, 3);
    formLayout->addRow(tr("&Partitions:"), this->sbPartitions);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Save"));
    this->btnCancel = new QPushButton(tr("&Don't Save"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    this->btnOk->setDefault(true);
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    this->btnCancel = new QPushButton(cancelIcon, tr("&Cancel"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(this->btnOk);
    buttonLayout->addSpacing(5);
    buttonLayout->addWidget(this->btnCancel);
    //buttonLayout
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedSize(QSize(500, 170));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Topic"));
    setWindowIcon(QIcon(":/topic"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &NewTopicWindow::handleNewTopic);
    connect(this->btnCancel, &QPushButton::clicked, this, &NewTopicWindow::close);
}

void NewTopicWindow::afterWindowActivated(const QVariant& _var)
{
    if (_var.canConvert<Namespace>())
    {
        this->m_Namespace = _var.value<Namespace>();
        if (!this->m_Namespace.authToken().isEmpty())
        {
            this->m_TopicService->setAuthToken(this->m_Namespace.authToken());
        }
    }
}

void NewTopicWindow::handleNewTopic(bool)
{
    if (this->tbTopicName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("topic is required."));
        this->tbTopicName->setFocus();
        return;
    }
    int partitions = this->sbPartitions->value();

    QString domain = this->rbPersistent->isChecked() ? QString("persistent") : QString("non-persistent");
    Topic topic(this->tbTopicName->text(), this->m_Namespace);
    topic.setDomain(domain);
    TopicStats stats;
    stats.setPartitions(partitions);
    topic.setStats(stats);
    HttpStatusCode error;
    this->m_TopicService->createTopic(topic, error);
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        emit completedNewTopic(QVariant::fromValue(topic));
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}
