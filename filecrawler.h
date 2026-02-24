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

    ScanResult scanFolder(const QString &path)
    {
        ScanResult result;
        result.success = false;

        QFileInfo dirInfo(path);
        if (!dirInfo.exists())
        {
            result.errorMessage = "Error: folder '" + path + "' does not exist";
            return result;
        }

        if (!dirInfo.isDir())
        {
            result.errorMessage = "Error: '" + path + "' is not a folder";
            return result;
        }

        if (!dirInfo.isReadable())
        {
            result.errorMessage = "Error: no read permission for '" + path + "'";
            return result;
        }

        m_rootPath = QDir(path).absolutePath();

        QDirIterator it(path,QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot | QDir::Hidden,QDirIterator::Subdirectories);

        while (it.hasNext())
        {
            it.next();
            QFileInfo entry = it.fileInfo();

            FileItem item;
            item.filePath = entry.absoluteFilePath();
            item.fileName = entry.fileName();
            item.relativePath = QDir(m_rootPath).relativeFilePath(entry.absoluteFilePath());
            item.size = entry.size();
            item.isDir = entry.isDir();
            item.isHidden = entry.isHidden();

            result.items.append(item);
        }

        result.success = true;
        return result;
    }

private:
    QString m_rootPath;
};

#endif
