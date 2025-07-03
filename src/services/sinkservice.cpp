#include "sinkservice.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QHttpPart>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QDebug>

#include "../constants.h"

QList<Sink> SinkService::sinks(const Namespace& _namespace) const
{
    QList<Sink> sinks;

    QStringList names = sinkNames(_namespace);
    Q_FOREACH (const QString& name, names)
    {
        Sink sink;
        sink.setNamespace(_namespace);
        sink.appendInfo(information(_namespace, name));
        sink.appendStatus(status(_namespace, name));
        sinks << sink;
    }

    return sinks;
}

void SinkService::create(const Sink& _sink, QFile* _file, HttpStatusCode& _code)
{
    QByteArray json = _sink.toJson();
    qDebug() << "Greate a new Sink request body: " << QString::fromLatin1(json) << Qt::endl;

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    //配置部分
    QHttpPart configPart;
    configPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    configPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sinkConfig\""));
    configPart.setBody(json);
    //文件部分
    QHttpPart filePart;
    QFileInfo fileInfo(_file->fileName());
    QMimeDatabase db;
    QMimeType type = db.mimeTypeForFile(fileInfo);
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(type.name()));
    QString header("form-data; name=\"data\"; filename=\"%1\"");
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(header.arg(fileInfo.fileName())));
    _file->open(QIODevice::ReadOnly);
    _file->setParent(multiPart);
    filePart.setBodyDevice(_file);
    multiPart->append(filePart);
    multiPart->append(configPart);

    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(PUT_NEW_SINK_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Greate a new Sink service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->post(url, multiPart, code);
    switch (code)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 400:
        _code.code = HttpStatusCode::StatusCode::BadRequest;
        _code.errorDesc = QString::fromLatin1(result);
        break;
    case 401:
        _code.code = HttpStatusCode::StatusCode::Unauthorized;
        _code.errorDesc = QString(tr("Client is not authorize to perform operation."));
        break;
    case 503:
        _code.code = HttpStatusCode::StatusCode::ServiceUnavailable;
        _code.errorDesc = QString(tr("Function worker service is now initializing. Please try again later."));
        break;
    case 500:
        _code.code = HttpStatusCode::StatusCode::InternalServerError;
        _code.errorDesc = QString(tr("Internal Server Error."));
        break;
    }

    qDebug() << "Greate a new Sink response result: " << QString::fromLatin1(result) << ", code: " << code << Qt::endl;
}

void SinkService::update(const Sink& _sink, QFile* _file, HttpStatusCode& _code)
{
    QByteArray json = _sink.toJson();
    qDebug() << "Update Sink request body: " << QString::fromLatin1(json) << Qt::endl;

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    //配置部分
    QHttpPart configPart;
    configPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/json"));
    configPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"sinkConfig\""));
    configPart.setBody(json);
    multiPart->append(configPart);
    if (_file != Q_NULLPTR)
    {
        QHttpPart filePart;
        QFileInfo fileInfo(_file->fileName());
        QMimeDatabase db;
        QMimeType type = db.mimeTypeForFile(fileInfo);
        //filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/java-archive"));
        filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(type.name()));
        QString header("form-data; name=\"data\"; filename=\"%1\"");
        filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant(header.arg(fileInfo.fileName())));
        _file->open(QIODevice::ReadOnly);
        _file->setParent(multiPart);
        filePart.setBodyDevice(_file);
        multiPart->append(filePart);
    }

    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(PUT_SINK_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Updae a Sink service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->put(url, multiPart, code);
    switch (code)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 400:
        _code.code = HttpStatusCode::StatusCode::BadRequest;
        _code.errorDesc = QString::fromLatin1(result);
        break;
    case 401:
        _code.code = HttpStatusCode::StatusCode::Unauthorized;
        _code.errorDesc = QString(tr("Client is not authorize to perform operation."));
        break;
    case 403:
        _code.code = HttpStatusCode::StatusCode::Forbidden;
        _code.errorDesc = QString(tr("The requester doesn't have admin permissions."));
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("The Pulsar Sink doesn't exist."));
        break;
    case 503:
        _code.code = HttpStatusCode::StatusCode::ServiceUnavailable;
        _code.errorDesc = QString(tr("Function worker service is now initializing. Please try again later."));
        break;
    case 500:
        _code.code = HttpStatusCode::StatusCode::InternalServerError;
        _code.errorDesc = QString(tr("Internal Server Error."));
        break;
    }

    qDebug() << "Updae a Sink response result: " << QString::fromLatin1(result) << ", code: " << code << Qt::endl;
}

void SinkService::remove(const Sink& _sink, HttpStatusCode& _code)
{
    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(DELETE_SINK_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Delete a Sink service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->deleteResource(url, statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("The Pulsar Sink doesn't exist."));
        break;
    case 400:
        _code.code = HttpStatusCode::StatusCode::BadRequest;
        _code.errorDesc = QString(tr("Invalid request."));
        break;
    case 401:
        _code.code = HttpStatusCode::StatusCode::Unauthorized;
        _code.errorDesc = QString(tr("Client is not authorize to perform operation."));
        break;
    case 408:
        _code.code = HttpStatusCode::StatusCode::RequestTimeout;
        _code.errorDesc = QString(tr("Request timeout."));
        break;
    case 503:
        _code.code = HttpStatusCode::StatusCode::ServiceUnavailable;
        _code.errorDesc = QString(tr("Function worker service is now initializing. Please try again later."));
        break;
    case 500:
        _code.code = HttpStatusCode::StatusCode::InternalServerError;
        _code.errorDesc = QString(tr("Internal Server Error."));
        break;
    }

    qDebug() << "Delete a Sink response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
}

void SinkService::start(const Sink& _sink, HttpStatusCode& _code)
{
    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(START_SINK_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Start a Sink service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->post(url, QByteArray(), statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("The Pulsar Sink does not exist."));
        break;
    case 400:
        _code.code = HttpStatusCode::StatusCode::BadRequest;
        _code.errorDesc = QString(tr("Invalid request."));
        break;
    case 401:
        _code.code = HttpStatusCode::StatusCode::Unauthorized;
        _code.errorDesc = QString(tr("Client is not authorize to perform operation."));
        break;
    case 500:
        _code.code = HttpStatusCode::StatusCode::InternalServerError;
        _code.errorDesc = QString(tr("Internal server error."));
        break;
    case 503:
        _code.code = HttpStatusCode::StatusCode::ServiceUnavailable;
        _code.errorDesc = QString(tr("Function worker service is now initializing. Please try again later."));
        break;
    }

    qDebug() << "Start a Sink response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
}

void SinkService::stop(const Sink& _sink, HttpStatusCode& _code)
{
    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(STOP_SINK_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Stop a Sink service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->post(url, QByteArray(), statusCode);
    switch (statusCode)
    {
    case 204:
        _code.code = HttpStatusCode::StatusCode::NoContent;
        break;
    case 404:
        _code.code = HttpStatusCode::StatusCode::NotFound;
        _code.errorDesc = QString(tr("The Pulsar Sink does not exist."));
        break;
    case 400:
        _code.code = HttpStatusCode::StatusCode::BadRequest;
        _code.errorDesc = QString(tr("Invalid request."));
        break;
    case 401:
        _code.code = HttpStatusCode::StatusCode::Unauthorized;
        _code.errorDesc = QString(tr("Client is not authorize to perform operation."));
        break;
    case 500:
        _code.code = HttpStatusCode::StatusCode::InternalServerError;
        _code.errorDesc = QString(tr("Internal server error."));
        break;
    case 503:
        _code.code = HttpStatusCode::StatusCode::ServiceUnavailable;
        _code.errorDesc = QString(tr("Function worker service is now initializing. Please try again later."));
        break;
    }

    qDebug() << "Stop a Sink response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
}

QList<FunctionInstance> SinkService::instances(const Sink& _sink) const
{
    QList<FunctionInstance> instances;
    QString path(_sink.getNamespace().tenant().cluster().functionUrl());
    path += this->m_Settings->value(GET_SINK_STATUS_PATH_KEY).toString();
    QUrl url(path.arg(_sink.getNamespace().tenant().name(), _sink.getNamespace().name(), _sink.name()));
    qDebug() << "Get Sink status service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->get(url, code);
    if (code == HttpStatusCode::StatusCode::OK)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(result, &error);
        if (error.error == QJsonParseError::ParseError::NoError)
        {
            QJsonObject root = doc.object();
            QJsonArray array = root["instances"].toArray();
            for (int i = 0, n = array.size(); i < n; ++i)
            {
                QJsonObject object = array[i].toObject();
                FunctionInstance instance;
                instance.setNamespace(_sink.getNamespace());
                instance.setName(_sink.name());
                instance.setInstanceId(object["instanceId"].toInt());
                QJsonObject status = object["status"].toObject();
                instance.setRunning(status["running"].toBool());
                instance.setReceivedNum(status["numReceivedFromSource"].toInt());
                instance.setWrittenNum(status["numWritten"].toInt());
                instance.setErrorNum(status["numSourceExceptions"].toInt());
                instances << instance;
            }
        }
    }

    qDebug() << "Get Source Sink response result: " << QString::fromLatin1(result) << Qt::endl;

    return instances;
}

QStringList SinkService::sinkNames(const Namespace& _namespace) const
{
    QStringList names;
    QString path(_namespace.tenant().cluster().functionUrl());
    path += this->m_Settings->value(GET_SINKS_PATH_KEY).toString();
    QUrl url(path.arg(_namespace.tenant().name(), _namespace.name()));
    qDebug() << "Get Sink names service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->get(url, code);
    if (code == HttpStatusCode::StatusCode::OK)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(result, &error);
        if (error.error == QJsonParseError::ParseError::NoError)
        {
            QJsonArray roots = doc.array();
            for (int i = 0, n = roots.size(); i < n; ++i)
            {
                names << roots[i].toString();
            }
        }
    }

    qDebug() << "Get Sink names response result: " << QString::fromLatin1(result) << ", code: " << code << Qt::endl;

    return names;
}

QByteArray SinkService::information(const Namespace& _namespace, const QString& _name) const
{
    QString path(_namespace.tenant().cluster().functionUrl());
    path += this->m_Settings->value(GET_SINK_INFO_PATH_KEY).toString();
    QUrl url(path.arg(_namespace.tenant().name(), _namespace.name(), _name));
    qDebug() << "Get Sink information service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->get(url, code);
    qDebug() << "Get Sink information response result: " << QString::fromLatin1(result) << ", code: " << code << Qt::endl;

    return result;
}

QByteArray SinkService::status(const Namespace& _namespace, const QString& _name) const
{
    QString path(_namespace.tenant().cluster().functionUrl());
    path += this->m_Settings->value(GET_SINK_STATUS_PATH_KEY).toString();
    QUrl url(path.arg(_namespace.tenant().name(), _namespace.name(), _name));
    qDebug() << "Get Sink status service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->get(url, code);
    qDebug() << "Get Sink status response result: " << QString::fromLatin1(result) << ", code: " << code << Qt::endl;

    return result;
}
