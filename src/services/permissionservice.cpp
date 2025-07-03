#include "permissionservice.h"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "../constants.h"

QStringList PermissionService::roles() const
{
    QStringList roles;
    return roles;
}

QStringList PermissionService::actions() const
{
    QStringList actions;
    actions << "produce" << "consume" << "functions";
    return actions;
}

QList<Role> PermissionService::roles(const Namespace& _namespace, HttpStatusCode& _error) const
{
    QList<Role> roles;
    QString path(_namespace.tenant().cluster().adminUrl());
    path = path.append(this->m_Settings->value(GET_NAMESPACE_PERMISSIONS_PATH_KEY).toString());
    QUrl url(path.arg(_namespace.tenant().name(), _namespace.name()));
    qDebug() << "Get Namespace Permissions service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray json = this->m_Client->get(url, code);
    if (code == 200)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(json, &error);
        if (error.error == QJsonParseError::ParseError::NoError)
        {
            QJsonObject root = doc.object();
            QJsonObject::const_iterator it;
            for (it = root.constBegin(); it != root.constEnd(); ++it)
            {
                QStringList actions;
                //foreach (QJsonValue v, it.value().toArray())
                //{
                //    actions << v.toString();
                //}
                QJsonArray array = it.value().toArray();
                for (const auto& v : std::as_const(array))
                {
                    actions << v.toString();
                }
                Role role;
                role.setNamespace(_namespace);
                role.setName(it.key());
                role.setPermissions(actions);
                roles << role;
            }
            _error.code = HttpStatusCode::StatusCode::OK;
        }
    }
    else
    {
        switch (code)
        {
        case 403:
            _error.code = HttpStatusCode::StatusCode::Forbidden;
            _error.errorDesc = QString(tr("Don't have admin permission."));
            break;
        case 404:
            _error.code = HttpStatusCode::StatusCode::NotFound;
            _error.errorDesc = QString(tr("Tenant or cluster or namespace doesn't exist."));
            break;
        case 409:
            _error.code = HttpStatusCode::StatusCode::Conflict;
            _error.errorDesc = QString(tr("Namespace is not empty."));
            break;
        }
    }

    qDebug() << "Get Namespace Permissions response result: " << QString::fromLatin1(json) << Qt::endl;

    return roles;
}

void PermissionService::grant(const Role& _role, HttpStatusCode& _code)
{
    if (exists(_role))
    {
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("Permission already exists."));
        return;
    }
    QString path(_role.getNamespace().tenant().cluster().adminUrl());
    path = path.append(this->m_Settings->value(GRANT_NAMESPACE_PERMISSION_PATH_KEY).toString());
    QUrl url(path.arg(_role.getNamespace().tenant().name(), _role.getNamespace().name(), _role.name()));
    qDebug() << "Grant Namespace Permission service url: " << url.toString() << Qt::endl;

    QByteArray body = _role.toJson();
    qDebug() << "Grant Permission request body: " << QString::fromLatin1(body) << Qt::endl;

    int statusCode;
    QByteArray json = this->m_Client->post(url, body, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("Tenant or cluster or namespace doesn't exist."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("Don't have admin permission."));
        break;
    case 409:
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("Concurrent modification."));
        break;
    case 501:
        _code.code = HttpStatusCode::StatusCode::NotImplemented;
        _code.errorDesc = QString(tr("Authorization is not enabled."));
        break;
    }

    qDebug() << "Grant Permission response result: " << QString::fromLatin1(json) << Qt::endl;
}

void PermissionService::revoke(const Role& _role, HttpStatusCode& _code)
{
    QString path(_role.getNamespace().tenant().cluster().adminUrl());
    path += this->m_Settings->value(REVOKE_NAMESPACE_PERMISSION_PATH_KEY).toString();
    QUrl url(path.arg(_role.getNamespace().tenant().name(), _role.getNamespace().name(), _role.name()));
    qDebug() << "Revoke Namespace Permission service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray json = this->m_Client->deleteResource(url, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("Tenant or cluster or namespace doesn't exist."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("Don't have admin permission."));
        break;
    }

    qDebug() << "Revoke Permission response result: " << QString::fromLatin1(json) << Qt::endl;
}

bool PermissionService::exists(const Role& _role) const
{
    bool exist = false;
    HttpStatusCode error;
    QList<Role> roles = this->roles(_role.getNamespace(), error);
    if (error.code == HttpStatusCode::StatusCode::OK)
    {
        foreach (Role role, roles)
        {
            if (role.name() == _role.name())
            {
                return true;
            }
        }
    }
    return exist;
}
