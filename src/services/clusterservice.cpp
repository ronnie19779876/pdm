#include "clusterservice.h"

#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QSettings>
#include <QDir>
#include <QCoreApplication>
#include <QDebug>

#include "../constants.h"
#include "../cluster.h"

ClusterService::ClusterService(QObject* parent) : BaseService(parent) {}

QStringList ClusterService::clusters(const Cluster& _cluster) const
{
    QStringList clusters;
    QString path(_cluster.adminUrl());
    path = path.append(this->m_Settings->value(GET_CLUSTERS_PATH_KEY).toString());
    QUrl url(path);
    qDebug() << "Get the list of all the Pulsar clusters service url: " << url.toString() << Qt::endl;

    QByteArray result = this->m_Client->get(url);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonArray roots = doc.array();
        for (int i = 0; i < roots.size(); ++i)
        {
            clusters << roots[i].toString();
        }
        qDebug() << "Get the list of all the Pulsar clusters response result: " << QString::fromLatin1(result) << Qt::endl;
    }
    return clusters;
}

bool ClusterService::exist(const QString& _name) const
{
    bool ok = false;
    if (this->m_Clusters.size() > 0)
    {
        Q_FOREACH (const Cluster& cluster, this->m_Clusters)
        {
            if (cluster.name() == _name)
            {
                ok = true;
                break;
            }
        }
    }
    return ok;
}

void ClusterService::modifyCluster(const Cluster& _cluster)
{
    removeCluster(_cluster);
    this->m_Clusters << _cluster;
}

void ClusterService::removeCluster(const Cluster& _cluster)
{
    Q_FOREACH (const Cluster& cluster, this->m_Clusters)
    {
        if (cluster.name() == _cluster.name())
        {
            this->m_Clusters.removeOne(cluster);
        }
    }
}

void ClusterService::write()
{
    QDir dir(QCoreApplication::applicationDirPath());
    QFile file(dir.absoluteFilePath("etc/clusters.dat"));
    if (file.open(QIODevice::WriteOnly))
    {
        QVariantList roots;
        Q_FOREACH (const Cluster& cluster, this->m_Clusters)
        {
            roots << cluster.toVariant();
        }
        QJsonDocument doc = QJsonDocument::fromVariant(roots);
        QByteArray json = doc.toJson(QJsonDocument::Compact);

        qDebug() << "Write clusters json: " << QString::fromLatin1(json) << Qt::endl;

        file.write(json);
        file.close();
    }
}

QString ClusterService::brokerServiceUrl(const Cluster& _cluster, const QString& _name) const
{
    QString path(_cluster.adminUrl());
    path = path.append(this->m_Settings->value(GET_BROKER_SRV_PATH_KEY).toString()).arg(_name);
    QUrl url(path);
    qDebug() << "Get the configuration for the specified cluster url: " << url.toString() << Qt::endl;

    QByteArray result = this->m_Client->get(url);
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(result, &error);
    if (error.error == QJsonParseError::ParseError::NoError)
    {
        QJsonObject root = doc.object();
        QString result(root["brokerServiceUrl"].toString());
        qDebug() << "Get the configuration for the specified cluster response result: " << result << Qt::endl;
        return result;
    }
    return QString("");
}

void ClusterService::read()
{
    //read data from binary file, its format is JSON.
    QDir dir(QCoreApplication::applicationDirPath());
    QFile file(dir.absoluteFilePath("etc/clusters.dat"));
    if (file.exists())
    {
        if (file.open(QIODevice::ReadOnly))
        {
            QJsonParseError error;
            QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &error);
            if (error.error == QJsonParseError::ParseError::NoError)
            {
                qDebug() << "Read clusters json: " << QString::fromLatin1(doc.toJson(QJsonDocument::Compact)) << Qt::endl;

                QJsonArray roots = doc.array();
                for (int i = 0; i < roots.size(); ++i)
                {
                    QJsonObject object = roots[i].toObject();
                    this->m_Clusters << Cluster::fromVariantMap(object.toVariantMap());
                }
            }
            else
            {
                qDebug() << "parse JSON error: " << error.error << Qt::endl;
            }
        }
        file.close();
    }
    emit completeRead();
}
