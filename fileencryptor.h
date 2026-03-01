#ifndef FILEENCRYPTOR_H
#define FILEENCRYPTOR_H

#include <QString>
#include <QDebug>

#include <cryptlib.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <osrng.h>
#include <sha.h>
#include <pwdbased.h>

struct FileResult {
    bool success = false;
    QString errorMessage;
    qint64 bytesProcessed = 0;
    QString outputPath;
};

class FileEncryptor {
public:
    FileEncryptor();
    ~FileEncryptor();

    FileResult encryptFile(const QString &inputPath,const QString &outputPath,const QString &password);

    void setProgressCallback(void (*callback)(int)) { m_progressCallback = callback; }

private:
    void (*m_progressCallback)(int) = nullptr;

    CryptoPP::SecByteBlock generateSalt(size_t size = 16);
    CryptoPP::SecByteBlock generateIV(size_t size = CryptoPP::AES::BLOCKSIZE);
    CryptoPP::SecByteBlock deriveKey(const QString &password,const CryptoPP::SecByteBlock &salt,size_t keySize = CryptoPP::AES::MAX_KEYLENGTH);
};

#endif // FILEENCRYPTOR_H
