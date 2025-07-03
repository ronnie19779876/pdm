#include "lastcommitmessagewindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QIcon>
#include <QComboBox>
#include <QTextEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QMessageBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QTabWidget>

#include "../pulsarmessage.h"
#include "../services/topicservice.h"

LastCommitMessageWindow::LastCommitMessageWindow(QWidget* _parent) : QDialog(_parent), m_TopicService(new TopicService(this)), twMessages(new QTableWidget(this))
{
    QVBoxLayout* layout = new QVBoxLayout;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblTopicName = new QLabel;
    this->cbPartitions = new QComboBox;
    this->lblMessageId = new QLabel;
    this->sbNumber = new QSpinBox;
    this->sbNumber->setRange(1, 100);
    this->sbNumber->setSingleStep(1);
    formLayout->addRow(tr("&Topic Name:"), lblTopicName);
    formLayout->addRow(tr("Partition:"), this->cbPartitions);
    formLayout->addRow(tr("&MessageId:"), this->lblMessageId);
    formLayout->addRow(tr("&Number of Messages:"), this->sbNumber);

    QStringList header;
    header << tr("LedgerId") << tr("EntryId") << tr("Message Key") << tr("Properties Count") << tr("Body Length");
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
    formLayout->addRow(new QLabel("Messages:"));
    formLayout->addRow(this->twMessages);

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
    formLayout->addRow(tabs);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
#ifdef Q_OS_MACOS
    this->btnGet = new QPushButton(tr("&Get it"));
    QPushButton* btnCancel = new QPushButton(tr("&Close"));
#else
    const QIcon okIcon = QIcon::fromTheme("emblem-default", QIcon(":/ok"));
    this->btnGet = new QPushButton(okIcon, tr("&Get it"));
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
    setFixedHeight(620);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("View Last Commit Message"));
    setWindowIcon(QIcon(":/images/message.ico"));
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(this->cbPartitions, &QComboBox::currentTextChanged, this, &LastCommitMessageWindow::handleCurrentIndexChanged);
    connect(this->btnGet, &QPushButton::clicked, this, &LastCommitMessageWindow::handleGetMessages);
    connect(btnCancel, &QPushButton::clicked, this, &LastCommitMessageWindow::close);
    connect(this->twMessages, &QTableWidget::itemSelectionChanged, this, &LastCommitMessageWindow::handleItemSelectionChanged);
    connect(this->cbSchema, &QComboBox::currentTextChanged, this, &LastCommitMessageWindow::handleCurrentTextChanged);
}

void LastCommitMessageWindow::afterWindowActivated(const QVariant& _var)
{
    if (_var.canConvert<Topic>())
    {
        this->m_topic = _var.value<Topic>();
        if (!this->m_topic.authToken().isEmpty())
        {
            this->m_TopicService->setAuthToken(this->m_topic.authToken());
        }
        this->lblTopicName->setText(QString("%1://%2/%3/%4").arg(m_topic.domain(), m_topic.getNamespace().tenant().name(), m_topic.getNamespace().name(), m_topic.name()));
        int partitions = this->m_topic.stats().partitions();
        if (partitions > 0)
        {
            for (int i = 0, n = partitions; i < n; i++)
            {
                this->cbPartitions->addItem(QString::number(i));
            }
        }
        else
        {
            handleCurrentIndexChanged(QString());
        }
    }
    QStringList schemas;
    schemas << "Text" << "Hex" << "Json";
    this->cbSchema->addItems(schemas);
}

void LastCommitMessageWindow::handleGetMessages()
{
    QVariant object = this->lblMessageId->property(QString("messageId").toLatin1());
    if (object.canConvert<Message>())
    {
        Message message = object.value<Message>();
        int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
        QList<PulsarMessage> messages = this->m_TopicService->messages(m_topic, partitions, message.ledgerId(), message.entryId(), this->sbNumber->value());
        if (messages.size() > 0)
        {
            //this->teProperties->clear();
            this->twMessages->clearContents();
            this->twMessages->setRowCount(messages.size());
            for (int i = messages.size() - 1, n = 0; i >= n; i--)
            {
                PulsarMessage pm = messages[i];
                QTableWidgetItem* item1 = new QTableWidgetItem(QString::number(pm.ledgerId()));
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

                //this->teMessage->insertHtml(QString(tr("<b>********** MessageId:%1:%2 **********</b><br>")).arg(message.ledgerId()).arg(message.entryId() - i));
                //this->teMessage->insertHtml(QString(tr("<div style='color: #0008FF;'>%1</div><br>")).arg(messages.at(i)));
                //this->teMessage->insertHtml("<br>");
            }
        }
        else
        {
            QMessageBox::warning(this, "Warning", "No messages can be read.");
        }
    }
}

void LastCommitMessageWindow::handleCurrentIndexChanged(const QString& _text)
{
    Q_UNUSED(_text);
    Message message;
    int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
    this->m_TopicService->getLastMessageId(this->m_topic, partitions, message);
    this->lblMessageId->setText(QString("%1:%2").arg(message.ledgerId()).arg(message.entryId()));
    this->lblMessageId->setProperty(QString("messageId").toLatin1(), QVariant::fromValue(message));
}

void LastCommitMessageWindow::handleItemSelectionChanged()
{
    //this->teProperties->clear();
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

void LastCommitMessageWindow::handleCurrentTextChanged(const QString& _text)
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
