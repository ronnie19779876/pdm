#include "baseservice.h"

#include <QDir>
#include <QSettings>
#include <QCoreApplication>

#include "../constants.h"
#include "httpclient.h"

BaseService::BaseService(QObject* parent) : QObject(parent), m_Client(new HttpClient(this))
{
    QDir dir(QCoreApplication::applicationDirPath());
    this->m_Settings = new QSettings(dir.absoluteFilePath(INI_FILE), QSettings::IniFormat, this);
}
