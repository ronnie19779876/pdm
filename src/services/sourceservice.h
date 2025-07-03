#ifndef SOURCESERVICE_H
#define SOURCESERVICE_H

#include "baseservice.h"

#include <QFile>

#include "../source.h"

class SourceService : public BaseService
{
    Q_OBJECT

public:
    explicit SourceService(QObject* parent = nullptr) : BaseService(parent) {}

    QList<Source> sources(const Namespace& _namespace);
    Source& information(Source& _source);
    Source& status(Source& _source);
    void createSource(const Source& _source, QFile* _file, HttpStatusCode& _code);
    void updateSource(const Source& _source, QFile* _file, HttpStatusCode& _code);
    void deleteSource(const Source& _source, HttpStatusCode& _code);
    void startSource(const Source& _source, HttpStatusCode& _code);
    void stopSource(const Source& _source, HttpStatusCode& _code);
    QList<SourceInstance> instances(const Source& _source);
};

#endif // SOURCESERVICE_H
