#include "cryptomanager.h"
#include "scanresult.h"
#include "fileitem.h"

const int MAX_PASSWORD_LENGTH = 64;

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
    if (path.trimmed().isEmpty())
    {
        FileResult result;
        result.success = false;
        result.errorMessage = "Path must not be empty.";
        return result;
    }

    if (password.trimmed().isEmpty()) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password must not be empty.";
        return result;
    }

    if (password.length() > MAX_PASSWORD_LENGTH) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password's length can no be more than 64 characters.";
        return result;
    }
    return encryptor.encryptFile(path, password);
}

FileResult CryptoManager::decryptFile(const QString& path, const QString& password)
{
    if (path.trimmed().isEmpty())
    {
        FileResult result;
        result.success = false;
        result.errorMessage = "Path must not be empty.";
        return result;
    }
    if (password.trimmed().isEmpty()) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password must not be empty.";
        return result;
    }
    if (password.length() > MAX_PASSWORD_LENGTH) {
        FileResult result;
        result.success = false;
        result.errorMessage = "Password's length can no be more than 64 characters.";
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

BatchResult CryptoManager::processFolder(const QString& folderPath,const QString& password,bool encryptMode)
{
    BatchResult batchResult;

    if (password.trimmed().isEmpty()) {
        batchResult.success = false;
        batchResult.errors.append("Password must not be empty.");
        return batchResult;
    }

    if (password.length() > MAX_PASSWORD_LENGTH) {
        batchResult.success = false;
        batchResult.errors.append("Password's length can no be more than 64 characters.");
        return batchResult;
    }

    ScanResult scanResult = crawler.scanFolder(folderPath);

    if (!scanResult.success) {
        batchResult.success = false;
        batchResult.errors.append(scanResult.errorMessage);
        return batchResult;
    }

    batchResult.totalFiles = scanResult.items.size();
    const QList<FileItem>& items = scanResult.items;
    const int count = items.size();

    for (int i = 0; i < count; ++i)
    {
        const FileItem& item = items[i];

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
