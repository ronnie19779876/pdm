#ifndef SINKSERVICE_H
#define SINKSERVICE_H

#include "baseservice.h"

#include <QFile>

#include "../sink.h"
#include "../functioninstance.h"

class SinkService : public BaseService
{
    Q_OBJECT
public:
    explicit SinkService(QObject* parent = nullptr) : BaseService(parent) {}

    QList<Sink> sinks(const Namespace& _namespace) const;
    void create(const Sink& _sink, QFile* _file, HttpStatusCode& _code);
    void update(const Sink& _sink, QFile* _file, HttpStatusCode& _code);
    void remove(const Sink& _sink, HttpStatusCode& _code);
    void start(const Sink& _sink, HttpStatusCode& _code);
    void stop(const Sink& _sink, HttpStatusCode& _code);
    QList<FunctionInstance> instances(const Sink& _sink) const;

private:
    QStringList sinkNames(const Namespace& _namespace) const;
    QByteArray information(const Namespace& _namespace, const QString& _name) const;
    QByteArray status(const Namespace& _namespace, const QString& _name) const;

};

#endif // SINKSERVICE_H
