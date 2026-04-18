#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include "batchresult.h"
#include "fileresult.h"
#include <cryptlib.h>
#include <aes.h>
#include <QString>
#include <QByteArray>
#include <QList>

class CryptoManager
{
public:
    static CryptoManager& instance();

    FileResult encryptFile(const QString& path, const QString& password);
    FileResult decryptFile(const QString& path, const QString& password);

    BatchResult encryptFolder(const QString& folderPath, const QString& password);
    BatchResult decryptFolder(const QString& folderPath, const QString& password);

private:
    struct FileItem
    {
        QString filePath;
        QString fileName;
        QString relativePath;
        qint64 size = 0;
    };

    struct ScanResult
    {
        bool success = false;
        QString errorMessage;
        QList<FileItem> items;
    };

    CryptoManager();
    ~CryptoManager();

    CryptoManager(const CryptoManager&) = delete;
    CryptoManager& operator=(const CryptoManager&) = delete;
    CryptoManager(CryptoManager&&) = delete;
    CryptoManager& operator=(CryptoManager&&) = delete;

    BatchResult processFolder(const QString& folderPath,const QString& password,bool encryptMode);
    ScanResult scanFolder(const QString& path) const;
    bool isPasswordValid(const QString& password, QString& errorMessage) const;
    bool hasEncryptionSignature(const QString& filePath) const;

    CryptoPP::SecByteBlock generateSalt(size_t size = SALT_SIZE) const;
    CryptoPP::SecByteBlock generateIV(size_t size = IV_SIZE) const;
    CryptoPP::SecByteBlock deriveKey(const QString& password,const CryptoPP::SecByteBlock& salt,size_t keySize = CryptoPP::AES::MAX_KEYLENGTH) const;

    static const QByteArray ENCRYPTION_SIGNATURE;
    static const int SALT_SIZE = 16;
    static const int IV_SIZE = 12;
    static const int TAG_SIZE = 16;
    static const int MAX_PASSWORD_LENGTH = 64;
};

#endif // CRYPTOMANAGER_H
