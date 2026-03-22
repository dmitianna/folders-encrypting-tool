#ifndef SCANRESULT_H
#define SCANRESULT_H
#include <QString>
#include <QList>
#include "fileitem.h"

struct ScanResult
{
    bool success = false;
    QString errorMessage;
    QList<FileItem> items;
};
#endif // SCANRESULT_H
