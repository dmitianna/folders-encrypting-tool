#ifndef FILECRAWLER_H
#define FILECRAWLER_H

#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QList>
#include <QString>

class FileCrawler
{
public:
    struct FileItem
    {
        QString filePath;
        QString fileName;
        QString relativePath;
        qint64 size = 0;
        bool isHidden = false;
    };

    struct ScanResult
    {
        bool success = false;
        QString errorMessage;
        QList<FileItem> items;
    };

    FileCrawler();

    ScanResult scanFolder(const QString &path);

private:
    QString m_rootPath;
};

#endif // FILECRAWLER_H
