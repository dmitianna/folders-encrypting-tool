#ifndef FILEDECRYPTOR_H
#define FILEDECRYPTOR_H

#include <QString>
#include <QFile>
#include <QByteArray>

#include <cryptlib.h>
#include <aes.h>
#include <gcm.h>
#include <filters.h>
#include <osrng.h>
#include <sha.h>
#include <pwdbased.h>

#include "fileresult.h"

class FileDecryptor
{
public:
    FileDecryptor();

    FileResult decryptFile(const QString &filePath, const QString &password);

private:
    static const QByteArray ENCRYPTION_SIGNATURE;
    static const int SALT_SIZE = 16;
    static const int IV_SIZE = 12;
    static const int TAG_SIZE = 16;

    CryptoPP::SecByteBlock deriveKey(const QString &password,
                                     const CryptoPP::SecByteBlock &salt,
                                     size_t keySize = CryptoPP::AES::MAX_KEYLENGTH);

    bool hasEncryptionSignature(const QString &filePath);
    QString makeTempFilePath(const QString &filePath) const;
};

#endif // FILEDECRYPTOR_H
