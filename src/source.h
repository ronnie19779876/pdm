#ifndef SOURCE_H
#define SOURCE_H

#include "basefunction.h"

typedef QMap<QString, QString> SourceData;

class Source : public BaseFunction
{
public:
    explicit Source();
    Source(const Source& _other) : BaseFunction(_other) { *this = _other; }
    Source& operator=(const Source& _other);

    QString classname() const { return this->m_Classname; };
    void setClassname(const QString& _classname) { this->m_Classname = _classname; };

    QString topicName() const { return this->m_TopicName; };
    void setTopicName(const QString& _topicName) { this->m_TopicName = _topicName; };

    QByteArray toJson() const override;
    SourceData toData() const;

private:
    QString m_Classname;
    QString m_TopicName;

};

Q_DECLARE_METATYPE(Source);

typedef QMap<QString, QString> SourceInstanceData;

class SourceInstance : public Source
{
public:
    explicit SourceInstance() : Source(), m_InstanceId(-1), m_Running(false), m_ReceivedNum(0), m_WrittenNum(0), m_ErrorNum(0) {}
    SourceInstance(const Source& _source) : Source(_source), m_InstanceId(-1), m_Running(false), m_ReceivedNum(0), m_WrittenNum(0), m_ErrorNum(0) {}
    SourceInstance(const SourceInstance& _other): Source(_other) { *this = _other; }
    SourceInstance& operator=(const SourceInstance& _other);

    inline int instanceId() const { return this->m_InstanceId; }
    inline void setInstanceId(const int& _instanceId) { this->m_InstanceId = _instanceId; }

    inline bool isRunning() const { return this->m_Running; };
    inline void setRunning(bool _running) { this->m_Running = _running; };

    inline int receivedNum() const { return this->m_ReceivedNum; };
    inline void setReceivedNum(int _receivedNum) { this->m_ReceivedNum = _receivedNum; };

    inline int writtenNum() const { return this->m_WrittenNum; };
    inline void setWrittenNum(int _writtenNum) { this->m_WrittenNum = _writtenNum; };

    inline int errorNum() const { return this->m_ErrorNum; };
    inline void setErrorNum(int _errorNum) { this->m_ErrorNum = _errorNum; };

    SourceInstanceData toData() const;

private:
    int m_InstanceId;
    bool m_Running;
    int m_ReceivedNum;
    int m_WrittenNum;
    int m_ErrorNum;
};

Q_DECLARE_METATYPE(SourceInstance);

#endif // SOURCE_H
