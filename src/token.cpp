#include "token.h"

#include <QVariantMap>

Token::Token(): BaseModel(), m_AuthToken(QString()) {}

Token& Token::operator =(const Token& _other)
{
    BaseModel::operator=(_other);
    this->m_AuthToken = _other.authtoken();
    return *this;
}

bool Token::operator==(const Token& _other) const
{
    return BaseModel::operator==(_other) && this->m_AuthToken == _other.authtoken();
}

DataModel Token::toData() const
{
    DataModel model(BaseModel::toData());
    model["authToken"] = this->m_AuthToken;
    return model;
}

Token Token::fromVariantMap(const QVariantMap& _var)
{
    Token token;
    token.setName(_var["name"].toString());
    token.setAuthtoken(_var["authToken"].toString());
    return token;
}

QVariantMap Token::toQVariantMap() const
{
    QVariantMap map(BaseModel::toQVariantMap());
    map["authToken"] = QVariant(this->m_AuthToken);
    return map;
}
