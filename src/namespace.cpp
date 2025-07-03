#include "namespace.h"

#include <QVariant>

Namespace& Namespace::operator =(const Namespace& _other)
{
    BaseModel::operator=(_other);
    this->m_Tenant = _other.tenant();
    this->m_Role = _other.role();
    this->m_Auths = _other.auths();
    return *this;
}

QList<Namespace> Namespace::fromVariantList(const QVariantList& _vars, const Tenant& _tenant)
{
    QList<Namespace> namespaces;
    Q_FOREACH (QVariant _var, _vars)
    {
        Namespace ns;
        ns.setName(_var.toString().section("/", 1, 1));
        ns.setTenant(_tenant);
        namespaces << ns;
    }
    return namespaces;
}

QVariantMap Namespace::toQVariantMap() const
{
    QVariantMap map(BaseModel::toQVariantMap());
    QVariantMap authPolicies;
    QVariantMap namespaceAuth;
    namespaceAuth[this->m_Role] = QVariant(this->m_Auths);
    authPolicies["namespace_auth"] = QVariant(namespaceAuth);
    map["auth_policies"] = QVariant(authPolicies);
    return map;
}
