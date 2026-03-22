#include "fileencryptor.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <files.h>
#include <filters.h>
#include <gcm.h>
#include <osrng.h>

using namespace CryptoPP;

const QByteArray FileEncryptor::ENCRYPTION_SIGNATURE("\xDE\xAD\xBE\xEF\xCA\xFE\xBA\xBE", 8);

FileEncryptor::FileEncryptor()
{
}

SecByteBlock FileEncryptor::generateSalt(size_t size)
{
    AutoSeededRandomPool rng;
    SecByteBlock salt(size);
    rng.GenerateBlock(salt, salt.size());
    return salt;
}

SecByteBlock FileEncryptor::generateIV(size_t size)
{
    AutoSeededRandomPool rng;
    SecByteBlock iv(size);
    rng.GenerateBlock(iv, iv.size());
    return iv;
}

SecByteBlock FileEncryptor::deriveKey(const QString &password,
                                      const SecByteBlock &salt,
                                      size_t keySize)
{
    SecByteBlock key(keySize);
    const std::string pwd = password.toStdString();

    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
    pbkdf.DeriveKey(
        key, key.size(),
        0,
        reinterpret_cast<const byte*>(pwd.data()), pwd.size(),
        salt, salt.size(),
        100000
        );

    return key;
}

bool FileEncryptor::hasEncryptionSignature(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    if (file.size() < ENCRYPTION_SIGNATURE.size()) {
        file.close();
        return false;
    }

    const QByteArray signature = file.read(ENCRYPTION_SIGNATURE.size());
    file.close();

    return signature == ENCRYPTION_SIGNATURE;
}


FileResult FileEncryptor::encryptFile(const QString &filePath, const QString &password)
{
    FileResult result;

    if (password.trimmed().isEmpty()) {
        result.errorMessage = "Password must not be empty.";
        return result;
    }

    QFileInfo fileInfo(filePath);
    if (!fileInfo.exists()) {
        result.errorMessage = "File does not exist: " + filePath;
        return result;
    }

    if (!fileInfo.isFile()) {
        result.errorMessage = "Path is not a regular file: " + filePath;
        return result;
    }

    if (!fileInfo.isReadable()) {
        result.errorMessage = "File is not readable: " + filePath;
        return result;
    }

    QFileInfo dirInfo(fileInfo.absolutePath());
    if (!dirInfo.isWritable()) {
        result.errorMessage = "Target directory is not writable: " + fileInfo.absolutePath();
        return result;
    }

    if (!fileInfo.isWritable()) {
        result.errorMessage = "File is not writable: " + filePath;
        return result;
    }

    if (hasEncryptionSignature(filePath)) {
        result.skipped = true;
        result.errorMessage = "File is already encrypted.";
        return result;
    }

    try {
        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            result.errorMessage = "Failed to open input file: " + filePath;
            return result;
        }

        const QByteArray plainData = inputFile.readAll();
        inputFile.close();

        const SecByteBlock salt = generateSalt(SALT_SIZE);
        const SecByteBlock iv = generateIV(IV_SIZE);
        const SecByteBlock key = deriveKey(password, salt, AES::MAX_KEYLENGTH);

        GCM<AES>::Encryption encryption;
        encryption.SetKeyWithIV(key, key.size(), iv, iv.size());

        std::string encryptedData;
        AuthenticatedEncryptionFilter aef(
            encryption,
            new StringSink(encryptedData),
            false,
            TAG_SIZE
            );

        if (!plainData.isEmpty()) {
            aef.Put(reinterpret_cast<const byte*>(plainData.constData()),
                    static_cast<size_t>(plainData.size()));
        }
        aef.MessageEnd();

        QSaveFile outputFile(filePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            result.errorMessage = "Failed to open output file for write: " + filePath;
            return result;
        }

        if (outputFile.write(ENCRYPTION_SIGNATURE) != ENCRYPTION_SIGNATURE.size()) {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write file signature.";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(salt.data()), SALT_SIZE) != SALT_SIZE) {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write salt.";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE) != IV_SIZE) {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write IV.";
            return result;
        }

        if (!encryptedData.empty()) {
            const qint64 written = outputFile.write(encryptedData.data(),
                                                    static_cast<qint64>(encryptedData.size()));
            if (written != static_cast<qint64>(encryptedData.size())) {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write encrypted data.";
                return result;
            }
        }

        if (!outputFile.commit()) {
            result.errorMessage = "Failed to replace original file.";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainData.size();
        return result;
    }
    catch (const Exception &e) {
        result.errorMessage = "Crypto++ error: " + QString::fromStdString(e.what());
        return result;
    }
    catch (const std::exception &e) {
        result.errorMessage = "Error: " + QString::fromStdString(e.what());
        return result;
    }
}
