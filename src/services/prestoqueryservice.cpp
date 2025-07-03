#include "prestoqueryservice.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

#include "../constants.h"
#include "../topic.h"
#include "../table.h"

void PrestoQueryService::query(const Topic& _topic, Statement* _statement)
{
    QString path(_topic.getNamespace().tenant().cluster().prestoUrl());
    if (!path.isEmpty())
    {
        path = path.append(this->m_Settings->value(PRESTO_STATEMENT_PATH_KEY).toString());
        QUrl url(path);
        qDebug() << "Query Topic Data service url: " << url.toString() << Qt::endl;

        QString query("select * from pulsar.\"%1/%2\".\"%3\"");
        query = query.arg(_topic.getNamespace().tenant().name(), _topic.getNamespace().name(), _topic.name());
        if (!_statement->condition().isEmpty())
        {
            query =  query.append(" where %4").arg(_statement->condition());
        }
        qDebug() << "Query Topic Data request body: " << query << Qt::endl;

        int statusCode;
        QByteArray result = this->m_Client->post(url, query.toLatin1(), statusCode);

        if (statusCode == HttpStatusCode::StatusCode::OK)
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(result, &error);
            if (error.error == QJsonParseError::ParseError::NoError)
            {
                QJsonObject root = doc.object();
                _statement->setId(root["id"].toString());
                _statement->setNextUri(root["nextUri"].toString());
                QJsonObject stats = root["stats"].toObject();
                QueryState state;
                state.setState(stats["state"].toString());
                state.setElapsedTimeMillis(stats["elapsedTimeMillis"].toInt());
                state.setProcessedRows(stats["processedRows"].toInt());
                state.setProcessedBytes(stats["processedBytes"].toInt());
                if (state.state() == QString("FAILED"))
                {
                    QJsonObject error = root["error"].toObject();
                    _statement->setError(error["message"].toString());
                }
                _statement->setState(state);
            }
        }

        qDebug() << "Query Topic Data response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
    }
}

void PrestoQueryService::queryNext(Statement* _statement)
{
    QUrl url(_statement->nextUri());
    qDebug() << "Query Topic next data service url: " << url.toString() << Qt::endl;

    int statusCode;
    QByteArray result = this->m_Client->get(url, statusCode);
    if (statusCode == HttpStatusCode::StatusCode::OK)
    {
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(result, &error);
        if (error.error == QJsonParseError::ParseError::NoError)
        {
            QJsonObject root = doc.object();
            _statement->setId(root["id"].toString());
            _statement->setNextUri(root["nextUri"].toString());
            _statement->setCancelUri(root["partialCancelUri"].toString());
            QJsonObject stats = root["stats"].toObject();
            QueryState state;
            state.setState(stats["state"].toString());
            state.setElapsedTimeMillis(stats["elapsedTimeMillis"].toInt());
            state.setProcessedRows(stats["processedRows"].toInt());
            state.setProcessedBytes(stats["processedBytes"].toInt());
            if (state.state() == QString("FAILED"))
            {
                QJsonObject error = root["error"].toObject();
                _statement->setError(error["message"].toString());
            }
            _statement->setState(state);

            if (_statement->columns().size() <= 0)
            {
                QJsonArray columns = root["columns"].toArray();
                if (columns.size() > 0)
                {
                    _statement->clearColumns();
                    for (int i = 0, n = columns.size(); i < n; ++i)
                    {
                        QJsonObject columnObject = columns[i].toObject();
                        Column col;
                        col.setName(columnObject["name"].toString());
                        col.setType(columnObject["type"].toString());
                        _statement->addColumn(col);
                    }
                }
            }

            QJsonArray rows = root["data"].toArray();
            if (rows.size() > 0)
            {
                //_statement->clearResult();
                for (int i = 0, n = rows.size(); i < n; ++i)
                {
                    QJsonArray data = rows[i].toArray();
                    QStringList row;
                    for (int j = 0, m = data.size(); j < m; ++j)
                    {
                        if (data[j].isString())
                        {
                            row <<  data[j].toString();
                        }
                        else if (data[j].isBool())
                        {
                            row << (data[j].toBool() ? "true" : "false");
                        }
                        else if (data[j].isDouble())
                        {
                            row << QString::number(data[j].toDouble());
                        }
                        else if (data[j].isNull())
                        {
                            row << QString("");
                        }
                        else if (data[j].isUndefined())
                        {
                            row << QString("undefined");
                        }
                    }
                    _statement->addResult(row.join("\t"));
                }
            }
        }
    }

    qDebug() << "Query Topic next data response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
}

void PrestoQueryService::cancelQuery(Statement* _statement)
{
    if (!_statement->cancelUri().isEmpty())
    {
        QUrl url(_statement->cancelUri());
        qDebug() << "Cancel query Topic data service url: " << url.toString() << Qt::endl;

        int statusCode;
        QByteArray result = this->m_Client->deleteResource(url, statusCode);
        qDebug() << "Cancel query Topic data response result: " << QString::fromLatin1(result) << ", code: " << statusCode << Qt::endl;
    }
}
