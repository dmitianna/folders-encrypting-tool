#ifndef FILECRAWLER_H
#define FILECRAWLER_H

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QList>
#include <QString>
#include "scanresult.h"
class FileCrawler
{
public:
    FileCrawler();

    ScanResult scanFolder(const QString &path);

private:
    QString m_rootPath;
};

#endif // FILECRAWLER_H
