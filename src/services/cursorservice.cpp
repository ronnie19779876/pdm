#include "cursorservice.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>

#include "../constants.h"

Cursor CursorService::find(const Topic& _topic, const int& _partition, const QString& _name)
{
    Cursor cursor;
    QString topicName = _partition >= 0 ? QString("%1-partition-%2").arg(_topic.name()).arg(_partition) : _topic.name();
    QString path(_topic.getNamespace().tenant().cluster().adminUrl());
    path = path.append(this->m_Settings->value(GET_STORED_TOPIC_METADATA_KEY).toString());
    QUrl url(path.arg(_topic.getNamespace().tenant().name(), _topic.getNamespace().name(), topicName, _topic.domain()));
    qDebug() << "Find Cursor by name Service url: " << url.toString() << Qt::endl;

    int code;
    QByteArray result = this->m_Client->get(url, code);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonObject root = doc.object();
        QJsonObject cursors = root["cursors"].toObject();
        QJsonObject::const_iterator it;
        for (it = cursors.constBegin(); it != cursors.constEnd(); ++it)
        {
            //Log4Qt::Logger::logger("main")->debug("Cursor name: %1", it.key());
            if (it.key() == _name)
            {
                QJsonDocument _doc(it.value().toObject());
                cursor = Cursor::fromJson(_doc.toJson(), cursor);
                cursor.setName(it.key());
                break;
            }
        }
    }

    qDebug() << "Find Cursor by name response result: " << QString::fromLatin1(result) << Qt::endl;

    return cursor;
}
