#include "tokenservice.h"

#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QVariantList>
#include <QDebug>

#include "../qjsonwebtoken.h"

void TokenService::readTokens()
{
    //read data from binary file, its format is JSON.
    QDir dir(QCoreApplication::applicationDirPath());
    QFile file(dir.absoluteFilePath("etc/tokens.dat"));
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly))
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
            if (error.error == QJsonParseError::ParseError::NoError)
            {
                qDebug() << "Read tokens json: " << QString::fromLatin1(doc.toJson(QJsonDocument::Compact)) << Qt::endl;

                QJsonArray roots = doc.array();
                for (int i = 0; i < roots.size(); ++i)
                {
                    QJsonObject object = roots[i].toObject();
                    this->m_Tokens << Token::fromVariantMap(object.toVariantMap());
                }
            }
            else
            {
                qDebug() << "parse JSON error: " << error.error << Qt::endl;
            }
        }
        file.close();
    }
}

void TokenService::writeTokens()
{
    QDir dir(QCoreApplication::applicationDirPath());
    QFile file(dir.absoluteFilePath("etc/tokens.dat"));
    if (file.open(QIODevice::WriteOnly))
    {
        QVariantList roots;
        Q_FOREACH (const Token& token, this->m_Tokens)
        {
            roots << token.toVariant();
        }

        QJsonDocument doc = QJsonDocument::fromVariant(roots);
        QByteArray json = doc.toJson(QJsonDocument::Compact);

        qDebug() << "Write tokens json: " << QString::fromLatin1(json) << Qt::endl;
        file.write(json);
        file.close();
    }
}

bool TokenService::exist(const QString& _name) const
{
    bool ok = false;
    if (this->m_Tokens.size() > 0)
    {
        Q_FOREACH (const Token& token, this->m_Tokens)
        {
            if (token.name() == _name)
            {
                ok = true;
                break;
            }
        }
    }
    return ok;
}

QStringList TokenService::tokens() const
{
    QStringList tokes;
    Q_FOREACH (const Token& token, this->m_Tokens)
    {
        tokes << token.name();
    }
    return tokes;
}

Token TokenService::fromJsonWebToken(const QByteArray& _key, const QString& _subject) const
{
    Token token;
    token.setName(_subject);
    QJsonWebToken jwt(_key, _subject);
    bool ok;
    QString tokenString = jwt.token(&ok);
    if (ok)
    {
        token.setAuthtoken(tokenString);
    }
    return token;
}

bool TokenService::generateSecretKey(const QString& _filename) const
{
    QByteArray data = QJsonWebToken::key();
    if (data.length() == 32)
    {
        QFile file(_filename);
        file.open(QIODevice::WriteOnly);
        file.write(data);
        file.close();
        return true;
    }
    return false;
}

void TokenService::removeToken(const Token& _token)
{
    Q_FOREACH (const Token& token, this->m_Tokens)
    {
        if (token.name() == _token.name())
        {
            this->m_Tokens.removeOne(token);
        }
    }
}
