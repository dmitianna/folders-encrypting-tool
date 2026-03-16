#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include "fileencryptor.h"
#include "filedecryptor.h"
#include "filecrawler.h"
#include "batchresult.h"
#include <QString>
#include <QStringList>

class CryptoManager
{
public:
    static CryptoManager& instance();

    FileResult encryptFile(const QString& path, const QString& password);
    FileResult decryptFile(const QString& path, const QString& password);

    BatchResult encryptFolder(const QString& folderPath, const QString& password);
    BatchResult decryptFolder(const QString& folderPath, const QString& password);

private:
    CryptoManager();
    ~CryptoManager();

    CryptoManager(const CryptoManager&) = delete;
    CryptoManager& operator=(const CryptoManager&) = delete;

    BatchResult processFolder(const QString& folderPath,
                              const QString& password,
                              bool encryptMode);

private:
    FileEncryptor encryptor;
    FileDecryptor decryptor;
    FileCrawler crawler;
};

#endif // CRYPTOMANAGER_H
