#include "cluster.h"

#include <QVariantMap>

Cluster& Cluster::operator =(const Cluster& _other)
{
    Token::operator=(_other);
    this->m_AdminUrl = _other.adminUrl();
    this->m_FunctionUrl = _other.functionUrl();
    this->m_PrestoUrl = _other.prestoUrl();
    this->m_Status = _other.status();
    return *this;
}

bool Cluster::operator==(const Cluster& _other) const
{
    return Token::operator==(_other) && this->adminUrl() == _other.adminUrl()
           && this->functionUrl() == _other.functionUrl() && this->prestoUrl() == _other.prestoUrl()
           && this->status() == _other.status();
}

Cluster Cluster::fromVariantMap(const QVariantMap& _var)
{
    Cluster cluster;
    cluster.setName(_var["name"].toString());
    cluster.setAdminUrl(_var["serviceUrl"].toString());
    cluster.setFunctionUrl(_var["functionUrl"].toString());
    cluster.setPrestoUrl(_var["prestoUrl"].toString());
    cluster.setAuthtoken(_var["authToken"].toString());
    return cluster;
}

QVariantMap Cluster::toQVariantMap() const
{
    QVariantMap map(Token::toQVariantMap());
    map["serviceUrl"] = QVariant(this->m_AdminUrl);
    map["functionUrl"] = QVariant(this->m_FunctionUrl);
    map["prestoUrl"] = QVariant(this->m_PrestoUrl);
    return map;
}
