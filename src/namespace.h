#ifndef NAMESPACE_H
#define NAMESPACE_H

#include "tenant.h"

class Namespace: public BaseModel
{
public:
    explicit Namespace() : BaseModel() {}
    Namespace(const Tenant& _tenant): m_Tenant(_tenant) { Namespace(); }
    Namespace(const Namespace& _other) : BaseModel(_other), m_Tenant(_other.tenant()) {}
    Namespace& operator=(const Namespace& _other);
    friend inline bool operator==(const Namespace& _one, const Namespace& _other) { return _one.name() == _other.name() && _one.tenant() == _other.tenant(); }

    inline Tenant tenant() const { return this->m_Tenant; }
    inline void setTenant(const Tenant& _tenant) { this->m_Tenant = _tenant; }

    inline QString role() const { return this->m_Role; }
    inline void setRole(const QString& _role) { this->m_Role = _role; }

    inline QStringList auths() const { return this->m_Auths; }
    inline void setAuths(const QStringList& _auth) { this->m_Auths = _auth; }

    inline QString authToken() const { return this->m_Tenant.authToken(); }

    static QList<Namespace> fromVariantList(const QVariantList& _var, const Tenant& _tenant);

protected:
    virtual QVariantMap toQVariantMap() const override;

private:
    Tenant m_Tenant;
    QString m_Role;
    QStringList m_Auths;
};

#endif // NAMESPACE_H
