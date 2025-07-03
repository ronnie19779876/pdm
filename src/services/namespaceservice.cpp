#include "namespaceservice.h"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

#include "../constants.h"

QList<Namespace> NamespaceService::namespaces(const Tenant& _tenant) const
{
    QList<Namespace> namespaces;
    QString path(_tenant.cluster().adminUrl());
    path = path.append(this->m_Settings->value(GET_NAMESPACES_PATH_KEY).toString()).arg(_tenant.name());
    QUrl url(path);
    qDebug() << "Get the list of all the namespaces for a certain tenant service url: " << url.toString() << Qt::endl;

    QByteArray result = this->m_Client->get(url);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonArray array = doc.array();
        if (!array.isEmpty())
        {
            namespaces << Namespace::fromVariantList(array.toVariantList(), _tenant);
        }
    }

    qDebug() << "Get the list of all the namespaces for a certain tenant response result: " << QString::fromLatin1(result) << Qt::endl;

    return namespaces;
}

void NamespaceService::createNamespace(const Namespace& _namespace, HttpStatusCode& _code)
{
    QString path(_namespace.tenant().cluster().adminUrl());
    path = path.append(this->m_Settings->value(PUT_NEW_NAMESPACE_PATH_KEY).toString()).arg(_namespace.tenant().name(), _namespace.name());
    QUrl url(path);
    qDebug() << "Create a new namesapce service url: " << url.toString() << Qt::endl;

    QString json = _namespace.toJson();
    qDebug() << "Create a new namesapce request body: " << json << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->put(url, json.toLatin1(), statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 409:
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("Namespace already exists."));
        break;
    case 412:
        _code.code = HttpStatusCode::StatusCode::PreconditionFailed;
        _code.errorDesc = QString(tr("Namespace name is not valid."));
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("Tenant or cluster doesn't exist."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("Don't have admin permission."));
        break;
    }

    qDebug() << "Create a new namesapce response result: " << QString::fromLatin1(result) << Qt::endl;
}

void NamespaceService::deleteNamespace(const Namespace& _namespace, HttpStatusCode& _code)
{
    QString path(_namespace.tenant().cluster().adminUrl());
    path = path.append(this->m_Settings->value(DELETE_NAMESPACE_PATH_KEY).toString()).arg(_namespace.tenant().name(), _namespace.name());
    QUrl url(path);
    qDebug() << "Delete a namespace and all the topics under it service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->deleteResource(url, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 409:
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("Namespace is not empty."));
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

    qDebug() << "Delete a namespace and all the topics under it response result: " << QString::fromLatin1(result) << Qt::endl;
}
