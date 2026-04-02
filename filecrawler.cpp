#include "filecrawler.h"
#include "scanresult.h"
FileCrawler::FileCrawler()
{
}

ScanResult FileCrawler::scanFolder(const QString &path)
{
    ScanResult result;
    result.success = false;

    QFileInfo dirInfo(path);
    if (!dirInfo.exists()) {
        result.errorMessage = "Error: folder '" + path + "' does not exist";
        return result;
    }

    if (!dirInfo.isDir()) {
        result.errorMessage = "Error: '" + path + "' is not a folder";
        return result;
    }

    if (!dirInfo.isReadable()) {
        result.errorMessage = "Error: no read permission for '" + path + "'";
        return result;
    }


    if (dirInfo.isHidden()) {
        result.errorMessage = "Error: program can not working to hide files/folders.'" + path + " has attribute hide'";
        return result;
    }

    m_rootPath = QDir(path).absolutePath();

    QDirIterator it(path,QDir::Files | QDir::NoDotAndDotDot,QDirIterator::Subdirectories);

    while (it.hasNext()) {
        it.next();
        QFileInfo entry = it.fileInfo();

        if (entry.isSymLink())
        {
            continue;
        }
        if (entry.isHidden())
        {
            continue;
        }
        FileItem item;
        item.filePath = entry.absoluteFilePath();
        item.fileName = entry.fileName();
        item.relativePath = QDir(m_rootPath).relativeFilePath(entry.absoluteFilePath());
        item.size = entry.size();

        result.items.append(item);
    }

    result.success = true;
    return result;
}
