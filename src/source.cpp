#include "source.h"

Source::Source() : BaseFunction(), m_Classname(QString()), m_TopicName(QString()) {}

Source& Source::operator =(const Source& _other)
{
    BaseFunction::operator=(_other);
    this->m_Classname = _other.classname();
    this->m_TopicName = _other.topicName();
    return *this;
}

QByteArray Source::toJson() const
{
    QJsonObject root;
    root = createRoot(root);
    root["className"] = this->m_Classname;
    root["topicName"] = this->m_TopicName;
    QJsonObject configs;
    FunctionConfig::const_iterator it;
    FunctionConfig config = this->config();
    for (it = config.constBegin(); it != config.constEnd(); ++it)
        configs.insert(it.key(), it.value());
    root["configs"] = configs;
    QJsonDocument doc = QJsonDocument::fromVariant(root);
    return doc.toJson(QJsonDocument::JsonFormat::Compact);
}

SourceData Source::toData() const
{
    SourceData data;
    data["tenant"] = this->getNamespace().tenant().name();
    data["namesapces"] = this->getNamespace().name();
    data["name"] = this->name();
    data["topicName"] = this->topicName();
    data["className"] = this->classname();
    data["instanceNum"] = QString::number(this->instanceNum());
    data["runningNum"] = QString::number(this->runningNum());
    return data;
}

SourceInstance& SourceInstance::operator=(const SourceInstance& _other)
{
    Source::operator=(_other);
    this->m_InstanceId = _other.instanceId();
    this->m_Running = _other.isRunning();
    this->m_ReceivedNum = _other.receivedNum();
    this->m_WrittenNum = _other.writtenNum();
    this->m_ErrorNum = _other.errorNum();
    return *this;
}

SourceInstanceData SourceInstance::toData() const
{
    SourceInstanceData data;
    data["tenant"] = this->getNamespace().tenant().name();
    data["namesapces"] = this->getNamespace().name();
    data["name"] = this->name();
    data["id"] = QString::number(this->instanceId());
    data["status"] = this->isRunning() ? "true" : "false";
    data["received"] = QString::number(this->receivedNum());
    data["written"] = QString::number(this->writtenNum());
    data["errors"] = QString::number(this->errorNum());
    return data;
}
