#include "tenant.h"

#include <QVariantMap>

Tenant& Tenant::operator =(const Tenant& _other)
{
    BaseModel::operator=(_other);
    this->m_Cluster = _other.cluster();
    this->clusters() = _other.clusters();
    return *this;
}

QVariantMap Tenant::toQVariantMap() const
{
    QVariantMap map(BaseModel::toQVariantMap());
    map["allowedClusters"] = QVariant(this->m_Clusters);
    return map;
}
