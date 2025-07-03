#ifndef PRESTOQUERYSERVICE_H
#define PRESTOQUERYSERVICE_H

#include "baseservice.h"

class Topic;
class Statement;

class PrestoQueryService : public BaseService
{
    Q_OBJECT

public:
    explicit PrestoQueryService(QObject* parent = nullptr) : BaseService(parent) {}

    void query(const Topic& _topic, Statement* _statement);
    void queryNext(Statement* _statement);
    void cancelQuery(Statement* _statement);

private:
    QString m_ServiceHost;

};

#endif // PRESTOQUERYSERVICE_H
