#include "topicstoragewindow.h"

#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QTableWidget>
#include <QHeaderView>
#include <QPushButton>

#include "../services/topicservice.h"

TopicStorageWindow::TopicStorageWindow(QWidget* parent) : QDialog(parent), m_TopicService(new TopicService(this)), twSegments(new QTableWidget)
{
    QVBoxLayout* layout = new QVBoxLayout;
    QFormLayout* formLayout = new QFormLayout;
    formLayout->setFieldGrowthPolicy(QFormLayout::AllNonFixedFieldsGrow);
    this->lblStorageSize = new QLabel;
    this->lblEntries = new QLabel;
    this->lblSegments = new QLabel;
    this->cbPartitions = new QComboBox;
    formLayout->addRow(tr("&Storage Size:"), this->lblStorageSize);
    formLayout->addRow(tr("S&egments:"), this->lblSegments);
    formLayout->addRow(tr("&Number of Messages:"), this->lblEntries);
    formLayout->addRow(tr("Partition:"), this->cbPartitions);
    formLayout->addRow(new QLabel("Segments:"));

    QStringList header;
    header << tr("Ledger Id") << tr("Entries") << tr("Size") << tr("Status") << tr("Offload");
    this->twSegments->setColumnCount(header.length());
    this->twSegments->setHorizontalHeaderLabels(header);
    this->twSegments->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    this->twSegments->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->twSegments->horizontalHeader()->setStretchLastSection(true);
    this->twSegments->setSelectionMode(QAbstractItemView::SingleSelection);
    this->twSegments->verticalHeader()->setHidden(true);
    this->twSegments->setEditTriggers(QTableWidget::NoEditTriggers);
    this->twSegments->setFocusPolicy(Qt::NoFocus);
    this->twSegments->setContextMenuPolicy(Qt::CustomContextMenu);
    formLayout->addRow(this->twSegments);

    QHBoxLayout* buttonLayout = new QHBoxLayout;
    const QIcon cancelIcon = QIcon::fromTheme("window-close", QIcon(":/images/cancel.ico"));
    QPushButton* btnCancel = new QPushButton(cancelIcon, tr("&Close"));
    buttonLayout->setContentsMargins(0, 0, 0, 0);
    buttonLayout->addStretch();
    buttonLayout->addWidget(btnCancel);
    connect(btnCancel, &QPushButton::clicked, this, &TopicStorageWindow::close);

    layout->addLayout(formLayout);
    layout->addLayout(buttonLayout);

    setLayout(layout);
    setFixedWidth(800);
    setFixedHeight(460);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(tr("Topic Storage"));
    setWindowIcon(QIcon(":/images/storage.ico"));
    setWindowFlags(Qt::WindowCloseButtonHint);
}

void TopicStorageWindow::afterWindowActivated(const QVariant& _var)
{
    this->m_Variant = _var;
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        if (!topic.authToken().isEmpty())
        {
            this->m_TopicService->setAuthToken(topic.authToken());
        }
    }
    connect(this, &TopicStorageWindow::initialize, this, &TopicStorageWindow::handleInitialize);
    connect(this->cbPartitions, &QComboBox::currentTextChanged, this, &TopicStorageWindow::handleCurrentIndexChanged);
    emit initialize();
}

void TopicStorageWindow::handleInitialize()
{
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        int partitions = topic.stats().partitions();
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
}

void TopicStorageWindow::handleCurrentIndexChanged(const QString&)
{
    if (this->m_Variant.canConvert<Topic>())
    {
        Topic topic = this->m_Variant.value<Topic>();
        int partitions = this->cbPartitions->currentText().isEmpty() ? -1 : this->cbPartitions->currentText().toInt();
        TopicStorage storage;
        storage = this->m_TopicService->topicStorage(topic, partitions, storage);
        QString size = storage.storageSize() > 1024.0 ? QString::number(qRound(storage.storageSize() / 1024.0)).append(" KB") : QString::number(storage.storageSize()).append(" Bytes");
        this->lblStorageSize->setText(size);
        this->lblEntries->setText(QString::number(storage.entryNum()));
        this->lblSegments->setText(QString::number(storage.segmentNum()));
        connect(this, &TopicStorageWindow::loadTable, this, &TopicStorageWindow::handleLoadTable);
        emit loadTable(storage);
    }

}

void TopicStorageWindow::handleLoadTable(const TopicStorage& _storage)
{
    QStringList columns;
    columns << "ledgerId" << "entries" << "size"  << "status" << "offload";
    this->twSegments->clearContents();
    TopicSegments segments = _storage.segments();
    this->twSegments->setRowCount(segments.size());
    for (int i = 0, n = segments.length(); i < n ; i++)
    {
        TopicSegmentData data = segments[i].toData();
        for (int j = 0, m = columns.length(); j < m; j++)
        {
            QTableWidgetItem* item = new QTableWidgetItem(data[columns[j]]);
            this->twSegments->setItem(i, j, item);
            if (j == 0)
                item->setData(Qt::UserRole, QVariant::fromValue(segments[i]));
        }
    }
}
