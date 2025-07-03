#ifndef BASEMODEL_H
#define BASEMODEL_H

#include <QString>

typedef QMap<QString, QString> DataModel;

class BaseModel
{
public:
    explicit BaseModel(): m_Name(QString()) {}
    virtual ~BaseModel();
    BaseModel(const BaseModel& _other) { *this = _other; }
    BaseModel& operator=(const BaseModel& _other) { this->m_Name = _other.name(); return *this; }
    bool operator==(const BaseModel& _other) const { return this->m_Name == _other.name(); }

    void setName(const QString& _name) { this->m_Name = _name; }
    QString name() const { return this->m_Name; }

    QString toJson() const;
    virtual DataModel toData() const;
    QVariant toVariant() const;

protected:
    virtual QVariantMap toQVariantMap() const;
private:
    QString m_Name;
};

#endif // BASEMODEL_H
