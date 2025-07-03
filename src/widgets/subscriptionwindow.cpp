#include "subscriptionwindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLineEdit>
#include <QIcon>
#include <QPushButton>
#include <QMessageBox>

#include "../services/topicservice.h"

SubscriptionWindow::SubscriptionWindow(QWidget* parent) : QDialog(parent), m_TopicService(new TopicService(this))
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->tbName = new QLineEdit;
    formLayout->addRow(tr("&Subscription Name:"), this->tbName);

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
    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedSize(QSize(420, 100));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Subscription"));
    setWindowIcon(QIcon(":/subscription"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &SubscriptionWindow::handleNewSubscription);
    connect(this->btnCancel, &QPushButton::clicked, this, &SubscriptionWindow::close);
}

void SubscriptionWindow::afterWindowActivated(const QVariant& _var)
{
    this->m_Var = _var;
}

void SubscriptionWindow::handleNewSubscription()
{
    if (this->tbName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Subscription name is required."));
        this->tbName->setFocus();
        return;
    }
    HttpStatusCode error;
    Topic topic = this->m_Var.value<Topic>();
    this->m_TopicService->createSubscription(topic, this->tbName->text(), error);
    if (error.code == HttpStatusCode::StatusCode::NoContent)
    {
        emit completedNewSubscription();
        close();
    }
    else
    {
        QMessageBox::critical(this, tr("Error"), error.errorString());
    }
}
