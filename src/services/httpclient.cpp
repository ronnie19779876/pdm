#include "httpclient.h"

#include <QString>
#include <QEventLoop>
#include <QSslConfiguration>
#include <QSslSocket>
#include <QSsl>
#include <QHttpMultiPart>

HttpClient::HttpClient(QObject* parent) : QObject(parent), m_Client(new QNetworkAccessManager(this))
{
    //this->qnam->setTransferTimeout(500);
}

HttpClient::~HttpClient()
{
    if (m_Client)
    {
        delete m_Client;
        m_Client = Q_NULLPTR;
    }
}

QByteArray HttpClient::get(const QUrl& _url) const
{
    QNetworkRequest request(_url);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->get(request);
    QByteArray response =  waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    int code = status.toInt();
    if (code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = get(redirection.toUrl());

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::get(const QUrl& _url, int& _code) const
{
    QNetworkRequest request(_url);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->get(request);
    QByteArray response =  waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    if (_code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = get(redirection.toUrl(), _code);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::deleteResource(const QUrl& _url) const
{
    QNetworkRequest request(_url);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->deleteResource(request);
    QByteArray response =  waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    int code = status.toInt();
    if (code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = deleteResource(redirection.toUrl(), code);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::deleteResource(const QUrl& _url, int& _code) const
{
    QNetworkRequest request(_url);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->deleteResource(request);
    QByteArray response =  waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    if (_code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = deleteResource(redirection.toUrl(), _code);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::post(const QUrl& _url, const QByteArray& _body, const QString& _contentType) const
{
    QNetworkRequest request(_url);
    setupRequest(request, _contentType, _body.length());
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->post(request, _body);
    QByteArray response = waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    int code = status.toInt();
    if (code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = post(redirection.toUrl(), _body, _contentType);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::post(const QUrl& _url, const QByteArray& _body, int& _code, const QString& _contentType) const
{
    QNetworkRequest request(_url);
    setupRequest(request, _contentType, _body.length());
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->post(request, _body);
    QByteArray response = waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    if (_code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = post(redirection.toUrl(), _body, _code, _contentType);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::post(const QUrl& _url, QHttpMultiPart* _multiPart, int& _code)
{
    QNetworkRequest request(_url);
    setupRequest(request);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->post(request, _multiPart);
    QByteArray response = waitFor(reply);
    _multiPart->setParent(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::put(const QUrl& _url, const QByteArray& _body, const QString& _contentType) const
{
    QNetworkRequest request(_url);
    setupRequest(request, _contentType, _body.length());
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->put(request, _body);
    QByteArray response = waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    int code = status.toInt();
    if (code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = put(redirection.toUrl(), _body, _contentType);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::put(const QUrl& _url, const QByteArray& _body, int& _code, const QString& _contentType) const
{
    QNetworkRequest request(_url);
    setupRequest(request, _contentType, _body.length());
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->put(request, _body);
    QByteArray response = waitFor(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    if (_code == HttpStatusCode::StatusCode::TemporaryRedirect)
    {
        QVariant redirection = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
        response = put(redirection.toUrl(), _body, _code, _contentType);

        qDebug() << "RedirectionTargetAttribute: " << redirection.toString() << Qt::endl;
    }
    reply->deleteLater();
    return response;
}

QByteArray HttpClient::put(const QUrl& _url, QHttpMultiPart* _multiPart, int& _code)
{
    QNetworkRequest request(_url);
    setupRequest(request);
    if (!this->m_Token.isEmpty())
    {
        request.setRawHeader(QString("Authorization").toLatin1(), QString("Bearer ").append(this->m_Token).toLatin1());
    }
    QNetworkReply* reply = this->m_Client->put(request, _multiPart);
    QByteArray response = waitFor(reply);
    _multiPart->setParent(reply);
    QVariant status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    _code = status.toInt();
    reply->deleteLater();
    return response;
}

void HttpClient::setupRequest(QNetworkRequest& _req, const QString& _contentType, int _length) const
{
    QString scheme = _req.url().scheme();
    if (scheme == "https")
    {
        QSslConfiguration config;
        config.setPeerVerifyMode(QSslSocket::VerifyNone);
        config.setProtocol(QSsl::TlsV1_3);
        _req.setSslConfiguration(config);
    }
    if (_length > 0)
    {
        _req.setHeader(QNetworkRequest::ContentTypeHeader, _contentType);
        _req.setHeader(QNetworkRequest::ContentLengthHeader, _length);
        _req.setRawHeader(QString("X-Presto-User").toLatin1(), QString("test-user").toLatin1());
    }
}

void HttpClient::handleRequestError(QNetworkReply::NetworkError _error)
{
    qDebug() << "Http request has occurred error: " << _error/* == QNetworkReply::NetworkError::OperationCanceledError*/ << Qt::endl;
}

QByteArray HttpClient::waitFor(QNetworkReply* reply) const
{
    QEventLoop eventLoop;
    connect(reply, &QNetworkReply::errorOccurred, this, &HttpClient::handleRequestError);
    connect(this->m_Client, SIGNAL(finished(QNetworkReply*)), &eventLoop, SLOT(quit()));
    connect(reply, &QNetworkReply::errorOccurred, &eventLoop, &QEventLoop::quit);
    eventLoop.exec(); //block until finish
    return reply->readAll();
}
