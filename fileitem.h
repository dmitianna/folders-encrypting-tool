#ifndef FILEITEM_H
#define FILEITEM_H
#include <QString>

struct FileItem
{
    QString filePath;
    QString fileName;
    QString relativePath;
    qint64 size = 0;
};
#endif // FILEITEM_H
