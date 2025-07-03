#include "producer.h"

#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>

Producer& Producer::operator=(const Producer& _other)
{
    BaseModel::operator=(_other);
    this->m_Id = _other.id();
    this->m_Address = _other.address();
    this->m_MsgRateIn = _other.msgRateIn();
    this->m_MsgThroughputIn = _other.msgThroughputIn();
    this->m_AverageMsgSize = _other.averageMsgSize();
    this->m_ConnectedSince = _other.connectedSince();
    return *this;
}

bool Producer::operator==(const Producer& _other) const
{
    return BaseModel::operator==(_other) && this->m_Id == _other.id()
           && this->m_Address == _other.address();
}

Producer& Producer::fromJson(const QByteArray& _json, Producer& _producer)
{
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(_json, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonObject root = doc.object();
        _producer.setId(root["producerId"].toInt());
        _producer.setName(root["producerName"].toString());
        _producer.setMsgRateIn(root["msgRateIn"].toDouble());
        _producer.setMsgThroughputIn(root["msgThroughputIn"].toDouble());
        _producer.setAverageMsgSize(root["averageMsgSize"].toDouble());
        _producer.setAddress(root["address"].toString());
        _producer.setConnectedSince(root["connectedSince"].toString());
    }
    return _producer;
}

ProducerData Producer::toData() const
{
    ProducerData data;
    data.insert(QString("id"), QString::number(this->id()));
    data.insert(QString("name"), this->name());
    data.insert(QString("inRate"), QString::number(this->msgRateIn(), 'f', 2));
    data.insert(QString("inThroughout"), QString::number(this->msgThroughputIn(), 'f', 2));
    data.insert(QString("avgMsgSize"), QString::number(this->averageMsgSize(), 'f', 2));
    data.insert(QString("address"), this->address());
    data.insert(QString("since"), this->connectedSince());
    return data;
}
