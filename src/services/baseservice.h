#ifndef BASESERVICE_H
#define BASESERVICE_H

#include <QObject>
#include <QSettings>

#include "httpclient.h"

class BaseService : public QObject
{
    Q_OBJECT

public:
    explicit BaseService(QObject* parent = nullptr);

    inline void setAuthToken(const QString& _token) { this->m_Client->setToken(_token); }

signals:

protected:
    HttpClient* m_Client;
    QSettings* m_Settings;
};

#endif // BASESERVICE_H
