#include "basemodel.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QTextCodec>

BaseModel::~BaseModel() {}

QString BaseModel::toJson() const
{
    QJsonDocument doc = QJsonDocument::fromVariant(toQVariantMap());
    QTextCodec* codec = QTextCodec::codecForName("UTF-8");
    return QString(codec->toUnicode(doc.toJson(QJsonDocument::Compact)));
}

DataModel BaseModel::toData() const
{
    DataModel model;
    model["name"] = this->name();
    return model;
}

QVariant BaseModel::toVariant() const
{
    return toQVariantMap();
}

QVariantMap BaseModel::toQVariantMap() const
{
    QVariantMap root;
    root["name"] = this->m_Name;
    return root;
}

