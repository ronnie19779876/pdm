#ifndef TENANT_H
#define TENANT_H

#include <QStringList>

#include "cluster.h"

class Tenant: public BaseModel
{
public:
    explicit Tenant() : BaseModel() {}
    Tenant(const Cluster& _cluster): m_Cluster(_cluster) { Tenant(); }
    Tenant(const Tenant& _other) : BaseModel(_other), m_Cluster(_other.cluster()) {}
    Tenant& operator=(const Tenant&);
    friend inline bool operator==(const Tenant& _one, const Tenant& _other) { return _one.name() == _other.name(); }

    inline Cluster cluster() const { return this->m_Cluster; }
    inline void setCluster(const Cluster& _cluster) { this->m_Cluster = _cluster; }

    inline QStringList clusters() const { return this->m_Clusters; }

    inline void addCluster(const QString& _name) { this->m_Clusters << _name; }
    inline QString authToken() const { return this->m_Cluster.authtoken(); }

protected:
    virtual QVariantMap toQVariantMap() const override;

private:
    QStringList m_Clusters;
    Cluster m_Cluster;
};

#endif // TENANT_H
