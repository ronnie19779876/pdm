#include "tenantservice.h"

#include <QUrl>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>

#include "../constants.h"

QList<Tenant> TenantService::tenants(const Cluster& _cluster) const
{
    QList<Tenant> tenants;

    QString path(_cluster.adminUrl());
    path = path.append(this->m_Settings->value(GET_TENANTS_PATH_KEY).toString());
    QUrl url(path);
    qDebug() << "Get the list of existing tenants service url: " << url.toString() << Qt::endl;

    QByteArray result = this->m_Client->get(url);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonArray roots = doc.array();
        for (int i = 0, n = roots.size(); i < n; ++i)
        {
            Tenant tenant(_cluster);
            tenant.setName(roots[i].toString());
            tenants << tenant;
        }
    }

    qDebug() << "List tenants response result: " << QString::fromLatin1(result) << Qt::endl;

    return tenants;
}

void TenantService::createTenant(const Tenant& _tenant, HttpStatusCode& _code)
{
    QString path(_tenant.cluster().adminUrl());
    path = path.append(this->m_Settings->value(PUT_NEW_TENANT_PATH_KEY).toString()).arg(_tenant.name());
    QUrl url(path);
    qDebug() << "Create a new tenant service url: " << url.toString() << Qt::endl;

    QJsonDocument doc = QJsonDocument::fromVariant(_tenant.toVariant());
    QByteArray json = doc.toJson(QJsonDocument::JsonFormat::Compact);
    qDebug() << "Create a new tenant request body: " << QString::fromLatin1(json) << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->put(url, json, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 409:
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("Tenant already exists."));
        break;
    case 412:
        _code.code = HttpStatusCode::StatusCode::PreconditionFailed;
        _code.errorDesc = QString(tr("Clusters do not exist."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("The requester doesn't have admin permissions."));
        break;
    }

    qDebug() << "Create a new tenant Response result: " << QString::fromLatin1(result) << ", status code: " << statusCode << Qt::endl;
}

void TenantService::deleteTenant(const Tenant& _tenant, HttpStatusCode& _code)
{
    QString path(_tenant.cluster().adminUrl());
    path = path.append(this->m_Settings->value(DELETE_TENANT_PATH_KEY).toString()).arg(_tenant.name());
    QUrl url(path);
    qDebug() << "Delete a tenant and all namespaces and topics under it service path: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->deleteResource(url, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 409:
        _code.code = HttpStatusCode::StatusCode::Conflict;
        _code.errorDesc = QString(tr("The tenant still has active namespaces."));
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("Tenant does not exist."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("The requester doesn't have admin permissions."));
        break;
    }

    qDebug() << "Delete a tenant and all namespaces and topics under it Response result: " << QString::fromLatin1(result) << ", status code: " << statusCode << Qt::endl;
}
