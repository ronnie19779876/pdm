#ifndef TOKENSERVICE_H
#define TOKENSERVICE_H

#include "baseservice.h"

#include "../token.h"

class TokenService : public BaseService
{
    Q_OBJECT

public:
    explicit TokenService(QObject* parent = nullptr) : BaseService(parent) {}

    void readTokens();
    void writeTokens();
    bool exist(const QString&) const;
    void removeToken(const Token&);
    inline void addToken(const Token& _token) { this->m_Tokens << _token; }
    inline QList<Token> getTokens() const { return this->m_Tokens; }
    QStringList tokens() const;
    Token fromJsonWebToken(const QByteArray& _key, const QString& _subject) const;
    bool generateSecretKey(const QString& _filename) const;

private:
    QList<Token> m_Tokens;
};

#endif // TOKENSERVICE_H
