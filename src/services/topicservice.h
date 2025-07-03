#ifndef TOPICSERVICE_H
#define TOPICSERVICE_H

#include "baseservice.h"

#include "../topic.h"
#include "../pulsarmessage.h"

class TopicService : public BaseService
{
    Q_OBJECT

public:
    explicit TopicService(QObject* parent = nullptr) : BaseService(parent) {}

    QList<Topic> topics(const Namespace& _namespace) const;
    void createTopic(const Topic& _topic, HttpStatusCode& _code);
    void deleteTopic(const Topic& _topic, HttpStatusCode& _code);
    void getLastMessageId(const Topic& _topic, const int& _partition, Message& _message);
    QList<PulsarMessage> messages(const Topic& _topic, const int& _partition, const int& _ledgerId, const int& _entryId, const int& _num = 1) const;
    TopicStorage& topicStorage(const Topic& _topic, const int& _partition, TopicStorage& _storage);
    TopicStats overview(const Topic& _topic, const int& _partition) const;
    QList<PulsarMessage> messages(const Topic& _topic, const int& _partition, const QString& _subName, const int& _num = 1) const;
    void createSubscription(const Topic& _topic, const QString& _subName, HttpStatusCode& _code);
    void deleteSubscription(const Topic& _topic, const QString& _subName, HttpStatusCode& _code);

private:
    QList<Topic> partitionedTopics(const Namespace& _namespace) const;
    QList<Topic> nonePartitionedTopics(const Namespace& _namespace) const;
    TopicStats stats(const Topic& _topic) const;
    QString topicName(const QString& _fullname) const;
    QString domain(const QString& _fullname) const;
    int partitions(const Topic& _topic) const;

};

#endif // TOPICSERVICE_H
