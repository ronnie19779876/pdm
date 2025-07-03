#include "newclusterwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QIcon>
#include <QPushButton>
#include <QMessageBox>
#include <QSettings>
#include <QCoreApplication>

#include "../cluster.h"

NewClusterWindow::NewClusterWindow(QWidget* parent) : QDialog(parent), m_New(true)
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->tbName = new QLineEdit;
    this->tbServiceUrl =  new QLineEdit;
    this->tbFunctionUrl = new QLineEdit;
    this->tbPrestoUrl = new QLineEdit;
    this->tbAuthToken = new QLineEdit;
    formLayout->addRow(tr("&Cluster Name:"), this->tbName);
    formLayout->addRow(tr("&Admin Service URL:"), this->tbServiceUrl);
    formLayout->addRow(tr("&Function Service URL:"), this->tbFunctionUrl);
    formLayout->addRow(tr("&Presto Service URL:"), this->tbPrestoUrl);
    formLayout->addRow(tr("A&uthorization:"), this->tbAuthToken);

    QHBoxLayout* buttonLayout = new QHBoxLayout;

#ifdef Q_OS_MACOS
    this->btnOk = new QPushButton(tr("&Save"));
    this->btnCancel = new QPushButton(tr("&Don't Save"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok.png"));
    this->btnOk = new QPushButton(okIcon, tr("&Ok"));
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel.png"));
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
    setFixedSize(QSize(560, 230));
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("New Cluster"));
    setWindowIcon(QIcon(":/newcluster.png"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnOk, &QPushButton::clicked, this, &NewClusterWindow::handleNewCluster);
    connect(this->btnCancel, &QPushButton::clicked, this, &NewClusterWindow::close);
}

void NewClusterWindow::afterWindowActivated(const Cluster* _cluster)
{
    if (_cluster)
    {
        setWindowTitle(tr("Update Cluster"));
        this->tbName->setText(_cluster->name());
        this->tbName->setEnabled(false);
        this->tbServiceUrl->setText(_cluster->adminUrl());
        this->tbFunctionUrl->setText(_cluster->functionUrl());
        this->tbPrestoUrl->setText(_cluster->prestoUrl());
        this->tbAuthToken->setText(_cluster->authtoken());
        this->m_New = false;
    }
}

void NewClusterWindow::handleNewCluster()
{
    //check input items
    if (this->tbName->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Name input items must be entered."));
        this->tbName->setFocus();
        return;
    }
    else if (this->tbServiceUrl->text().isEmpty())
    {
        QMessageBox::critical(this, tr("Error"), tr("Admin Service URL input items must be entered."));
        this->tbServiceUrl->setFocus();
        return;
    }

    //todo:is check the url invalid?
    Cluster cluster;
    cluster.setName(this->tbName->text());
    cluster.setAdminUrl(this->tbServiceUrl->text());
    cluster.setFunctionUrl(this->tbFunctionUrl->text());
    cluster.setPrestoUrl(this->tbPrestoUrl->text());
    cluster.setAuthtoken(this->tbAuthToken->text());

    if (m_New)
    {
        emit completeNewCluster(cluster);
    }
    else
    {
        emit completeUpdateCluster(cluster);
    }
    close();
}
