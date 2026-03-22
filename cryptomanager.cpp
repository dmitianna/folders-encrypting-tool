#include "cryptomanager.h"

CryptoManager::CryptoManager()
{
}

CryptoManager::~CryptoManager()
{
}

CryptoManager& CryptoManager::instance()
{
    static CryptoManager instance;
    return instance;
}

FileResult CryptoManager::encryptFile(const QString& path, const QString& password)
{
    if (password.trimmed().isEmpty()) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password must not be empty.";
        return result;
    }

    return encryptor.encryptFile(path, password);
}

FileResult CryptoManager::decryptFile(const QString& path, const QString& password)
{
    if (password.trimmed().isEmpty()) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password must not be empty.";
        return result;
    }

    return decryptor.decryptFile(path, password);
}

BatchResult CryptoManager::encryptFolder(const QString& folderPath, const QString& password)
{
    return processFolder(folderPath, password, true);
}

BatchResult CryptoManager::decryptFolder(const QString& folderPath, const QString& password)
{
    return processFolder(folderPath, password, false);
}

BatchResult CryptoManager::processFolder(const QString& folderPath,
                                         const QString& password,
                                         bool encryptMode)
{
    BatchResult batchResult;

    if (password.trimmed().isEmpty()) {
        batchResult.success = false;
        batchResult.errors.append("Password must not be empty.");
        return batchResult;
    }

    FileCrawler::ScanResult scanResult = crawler.scanFolder(folderPath);

    if (!scanResult.success) {
        batchResult.success = false;
        batchResult.errors.append(scanResult.errorMessage);
        return batchResult;
    }

    batchResult.totalFiles = scanResult.items.size();
    const QList<FileCrawler::FileItem>& items = scanResult.items;
    const int count = items.size();

    for (int i = 0; i < count; ++i)
    {
        const FileCrawler::FileItem& item = items[i];

        FileResult fileResult;

        if (encryptMode)
        {
            fileResult = encryptFile(item.filePath, password);
        }
        else
        {
            fileResult = decryptFile(item.filePath, password);
        }

        if (fileResult.success)
        {
            batchResult.processedFiles++;
            batchResult.totalBytesProcessed += fileResult.bytesProcessed;
        }
        else if (fileResult.skipped)
        {
            batchResult.skippedFiles++;
        }
        else
        {
            batchResult.failedFiles++;
            batchResult.errors.append(item.filePath + " : " + fileResult.errorMessage);
        }
    }

    batchResult.success = (batchResult.failedFiles == 0);

    return batchResult;
}
