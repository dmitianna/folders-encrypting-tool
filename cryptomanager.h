#ifndef CRYPTOMANAGER_H
#define CRYPTOMANAGER_H

#include <cryptlib.h>
#include <aes.h>
#include <QString>
#include <QByteArray>
#include <QList>
#include <QStringList>

struct BatchResult
{
    bool success = false;
    int totalFiles = 0;
    int processedFiles = 0;
    int skippedFiles = 0;
    int failedFiles = 0;
    qint64 totalBytesProcessed = 0;
    QStringList errors;
};

struct FileResult
{
    bool success = false;
    bool skipped = false;
    QString errorMessage;
    qint64 bytesProcessed = 0;
};

class CryptoManager
{
public:
    static CryptoManager& instance();

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

    BatchResult processFolder(const QString& folderPath,const QString& password,bool shouldEncrypt);
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


    FileResult encryptFile(const QString& path, const QString& password);
    FileResult decryptFile(const QString& path, const QString& password);
};

#endif // CRYPTOMANAGER_H
