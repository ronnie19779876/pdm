#ifndef PERMISSIONSERVICE_H
#define PERMISSIONSERVICE_H

#include "baseservice.h"
#include "../role.h"

class PermissionService : public BaseService
{
    Q_OBJECT

public:
    explicit PermissionService(QObject* parent = nullptr) : BaseService(parent) {}

    QList<Role> roles(const Namespace&, HttpStatusCode&) const;
    QStringList roles() const;
    QStringList actions() const;
    void revoke(const Role&, HttpStatusCode&);
    void grant(const Role&, HttpStatusCode&);
    bool exists(const Role&) const;

};

#endif // PERMISSIONSERVICE_H
