#ifndef TOKEN_H
#define TOKEN_H

#include <QString>

#include "basemodel.h"

class Token: public BaseModel
{
public:
    explicit Token();
    Token(const Token& _other): BaseModel(_other), m_AuthToken(_other.authtoken()) {}
    Token& operator=(const Token& _other);
    bool operator==(const Token& _other) const;

    inline void setAuthtoken(const QString& _authToken) { this->m_AuthToken = _authToken; }
    inline QString authtoken() const { return this->m_AuthToken; }

    virtual DataModel toData() const override;
    static Token fromVariantMap(const QVariantMap& _var);

protected:
    virtual QVariantMap toQVariantMap() const override;

private:
    QString m_AuthToken;
};

#endif // TOKEN_H
