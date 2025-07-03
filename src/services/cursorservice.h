#ifndef CURSORSERVICE_H
#define CURSORSERVICE_H

#include "baseservice.h"

#include "../cursor.h"
#include "../topic.h"

class CursorService : public BaseService
{
    Q_OBJECT

public:
    explicit CursorService(QObject* parent = nullptr) : BaseService(parent) {}

    Cursor find(const Topic& _topic, const int& _partition, const QString& _name);

};

#endif // CURSORSERVICE_H
