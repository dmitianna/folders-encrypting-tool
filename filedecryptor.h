#ifndef FILEDECRYPTOR_H
#define FILEDECRYPTOR_H

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

class FileDecryptor {
public:
    FileDecryptor();
    ~FileDecryptor();

    FileResult decryptFile(const QString &inputPath,const QString &outputPath,const QString &password);

private:
    CryptoPP::SecByteBlock deriveKey(const QString &password,const CryptoPP::SecByteBlock &salt,size_t keySize = CryptoPP::AES::MAX_KEYLENGTH);
};

#endif // FILEDECRYPTOR_H
