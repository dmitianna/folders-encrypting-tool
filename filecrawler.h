#ifndef FILECRAWLER_H
#define FILECRAWLER_H

#include <QObject>
#include <QDir>
#include <QFileInfo>
#include <QDirIterator>
#include <QList>

class FileCrawler : public QObject
{
    Q_OBJECT

public:
    struct FileItem
    {
        QString filePath;
        QString fileName;
        QString relativePath;
        qint64 size = 0;
        bool isDir = false;
        bool isHidden = false;
    };

    struct ScanResult
    {
        bool success;
        QString errorMessage;
        QList<FileItem> items;
    };

    explicit FileCrawler(QObject *parent = nullptr) : QObject(parent) {}

private:
    QString m_rootPath;
};

#endif // FILECRAWLER_H
