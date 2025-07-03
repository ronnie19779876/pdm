#include "peekmessageswindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>
#include <QTextEdit>
#include <QComboBox>
#include <QDebug>

#include "../pulsarmessage.h"
#include "../services/cursorservice.h"
#include "../services/topicservice.h"

PeekMessagesWindow::PeekMessagesWindow(QWidget* parent) : QDialog(parent), m_TopicService(new TopicService(this)), m_CursorService(new CursorService(this))
{
    QVBoxLayout* layout = new QVBoxLayout;
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblTopicName = new QLabel;
    this->lblBacklog = new QLabel;
    this->sbNumber = new QSpinBox;
    this->sbNumber->setSingleStep(1);
    formLayout->addRow(tr("&Topic Name:"), this->lblTopicName);
    formLayout->addRow(tr("&Backlog:"), this->lblBacklog);
    formLayout->addRow(tr("&Number of Messages:"), this->sbNumber);
    layout->addLayout(formLayout);

    layout->addWidget(new QLabel(tr("Messages:")));
    QHBoxLayout* actionsLayout = new QHBoxLayout;
    this->btnPeek = new QPushButton(QIcon(":/images/ok.ico"), tr("&Peek Messages"));
    actionsLayout->addWidget(this->btnPeek);
    actionsLayout->addStretch();
    layout->addLayout(actionsLayout);
    QStringList header;
    header << tr("LedgerId") << tr("EntryId") << tr("Message Key") << tr("Properties Count") << tr("Body Length");
    this->twMessages = new QTableWidget(this);
    this->twMessages->setColumnCount(header.length());
    this->twMessages->setHorizontalHeaderLabels(header);
    this->twMessages->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twMessages->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twMessages->horizontalHeader()->setStretchLastSection(true);
    this->twMessages->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twMessages->verticalHeader()->setHidden(true);
    this->twMessages->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twMessages->setFocusPolicy(Qt::NoFocus);
    this->twMessages->setContextMenuPolicy(Qt::CustomContextMenu);
    layout->addWidget(this->twMessages);

    QTabWidget* tabs = new QTabWidget();
    this->teProperties = new QTextEdit();
    this->teProperties->setReadOnly(true);
    this->teKey = new QTextEdit();
    this->teKey->setReadOnly(true);
    this->teMessage = new QTextEdit;
    this->teMessage->setReadOnly(true);
    this->cbSchema = new QComboBox;

    QVBoxLayout* layoutProperties = new QVBoxLayout;
    layoutProperties->addWidget(this->teProperties);
    QVBoxLayout* layoutKey = new QVBoxLayout;
    layoutKey->addWidget(this->teKey);
    QVBoxLayout* layoutBody = new QVBoxLayout;
    QHBoxLayout* layoutSchema = new QHBoxLayout;
    layoutSchema->addWidget(new QLabel(tr("Message Schema:")));
    layoutSchema->addWidget(this->cbSchema);
    layoutSchema->addStretch();
    layoutBody->addLayout(layoutSchema);
    layoutBody->addWidget(this->teMessage);

    QWidget* tbProperties = new QWidget();
    QWidget* tbKey = new QWidget();
    QWidget* tbBody = new QWidget();
    tbProperties->setLayout(layoutProperties);
    tbKey->setLayout(layoutKey);
    tbBody->setLayout(layoutBody);
    tabs->addTab(tbProperties, tr("Message Properties"));
    tabs->addTab(tbKey, tr("Message Key"));
    tabs->addTab(tbBody, tr("Message Body"));
    layout->addWidget(tabs);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/cancel"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
#endif
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnCancel);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    resize(1024, 680);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Peek nth Messages"));
    setWindowIcon(QIcon(":/message"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->btnPeek, &QPushButton::clicked, this, &PeekMessagesWindow::handleInitialize);
    connect(btnCancel, &QPushButton::clicked, this, &PeekMessagesWindow::close);
    connect(this->sbNumber, SIGNAL(valueChanged(int)), this, SLOT(handleValueChanged(int)));
    connect(this, &PeekMessagesWindow::initialize, this, &PeekMessagesWindow::handleInitialize);
    connect(this->twMessages, &QTableWidget::itemSelectionChanged, this, &PeekMessagesWindow::handleItemSelectionChanged);
    connect(this->cbSchema, &QComboBox::currentTextChanged, this, &PeekMessagesWindow::handleCurrentTextChanged);
}

PeekMessagesWindow::~PeekMessagesWindow() {}

void PeekMessagesWindow::afterWindowActivated(const Topic& _topic, const Subscription& _subscription, const int& _partitions)
{
    this->m_Partitions = _partitions;
    this->m_Topic = _topic;
    if (!this->m_Topic.authToken().isEmpty())
    {
        this->m_TopicService->setAuthToken(this->m_Topic.authToken());
        this->m_CursorService->setAuthToken(this->m_Topic.authToken());
    }
    this->m_Subscription = _subscription;
    if (this->m_Subscription.msgBacklog() > 0)
    {
        this->sbNumber->setRange(1, this->m_Subscription.msgBacklog());
    }
    else
    {
        this->sbNumber->setEnabled(false);
        this->btnPeek->setEnabled(false);
    }
    this->lblTopicName->setText(this->m_Topic.formatName());
    this->lblBacklog->setText(QString::number(this->m_Subscription.msgBacklog()));
    QStringList schemas;
    schemas << "Text" << "Hex" << "Json";
    this->cbSchema->addItems(schemas);
    emit initialize();
}

void PeekMessagesWindow::handleInitialize()
{
    int backlog = this->m_Subscription.msgBacklog();
    if (backlog > 0)
    {
        Cursor cursor = m_CursorService->find(this->m_Topic, this->m_Partitions, this->m_Subscription.name());
        int ledgerId = cursor.deletePositionLedgerId();
        QList<PulsarMessage> messages = m_TopicService->messages(this->m_Topic, this->m_Partitions, this->m_Subscription.name(), this->sbNumber->value());
        this->twMessages->clearContents();
        this->twMessages->setRowCount(messages.size());
        for (int i = messages.size() - 1, n = 0; i >= n; i--)
        {
            PulsarMessage pm = messages[i];
            QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(ledgerId));
            item1->setTextAlignment(Qt::AlignCenter);
            this->twMessages->setItem(i, 0, item1);
            QTableWidgetItem* item2 = new QTableWidgetItem(QString::number(pm.entryId()));
            item2->setTextAlignment(Qt::AlignCenter);
            this->twMessages->setItem(i, 1, item2);
            QTableWidgetItem* item3 = new QTableWidgetItem(pm.key());
            this->twMessages->setItem(i, 2, item3);
            QTableWidgetItem* item4 = new QTableWidgetItem(QString::number(pm.properties().size()));
            item4->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
            this->twMessages->setItem(i, 3, item4);
            QTableWidgetItem* item5 = new QTableWidgetItem(QString::number(pm.body().length()));
            item5->setTextAlignment(Qt::AlignRight | Qt::AlignCenter);
            this->twMessages->setItem(i, 4, item5);
            item1->setData(Qt::UserRole, QVariant::fromValue(pm));
        }
    }
}

void PeekMessagesWindow::handleValueChanged(int _value)
{
    qDebug() << "SpinBox Value: " << _value << Qt::endl;
}

void PeekMessagesWindow::handleItemSelectionChanged()
{
    QTableWidgetItem* current = this->twMessages->currentItem();
    if (current)
    {
        int row = current->row();
        QTableWidgetItem* item = this->twMessages->item(row, 0);
        QVariant data = item->data(Qt::UserRole);
        if (data.canConvert<PulsarMessage>())
        {
            PulsarMessage message = data.value<PulsarMessage>();
            this->teProperties->setText(message.formatProperties());
            this->teKey->setText(message.key());
            this->teMessage->setProperty("body", data);
            this->teMessage->setText(message.toString());
            this->cbSchema->setCurrentIndex(0);
        }
    }
}

void PeekMessagesWindow::handleCurrentTextChanged(const QString& _text)
{
    QVariant data = this->teMessage->property("body");
    if (data.canConvert<PulsarMessage>())
    {
        PulsarMessage message = data.value<PulsarMessage>();
        if (_text == "Text")
        {
            this->teMessage->setText(message.toString());
        }
        else if (_text == "Json")
        {
            this->teMessage->setText(message.toJson());
        }
        else if (_text == "Hex")
        {
            this->teMessage->setText(message.toHex());
        }
    }
}
