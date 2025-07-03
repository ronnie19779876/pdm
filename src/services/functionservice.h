#ifndef FUNCTIONSERVICE_H
#define FUNCTIONSERVICE_H

#include "baseservice.h"

#include <QObject>
#include <QFile>

#include "../function.h"
#include "../functioninstance.h"

class FunctionService : public BaseService
{
    Q_OBJECT

public:
    explicit FunctionService(QObject* parent = nullptr) : BaseService(parent) {}

    QList<Function> functions(const Namespace& _namespace) const;
    QByteArray status(const Function& _function) const;
    QByteArray information(const Namespace& _namespace, const QString& _name) const;
    void createFunction(const Function& _function, QFile* _file, HttpStatusCode& _code);
    void updateFunction(const Function& _function, QFile* _file, HttpStatusCode& _code);
    void deleteFunction(const Function& _function, HttpStatusCode& _code);
    void startFunction(const Function& _function, HttpStatusCode& _code);
    void stopFunction(const Function& _function, HttpStatusCode& _code);
    QList<FunctionInstance> instances(const Function& _function) const;

};

#endif // FUNCTIONSERVICE_H
