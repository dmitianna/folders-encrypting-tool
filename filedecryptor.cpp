#include "filedecryptor.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <files.h>
#include <filters.h>
#include <gcm.h>
#include "pathutils.h"

using namespace CryptoPP;

const QByteArray FileDecryptor::ENCRYPTION_SIGNATURE("\xDE\xAD\xBE\xEF\xCA\xFE\xBA\xBE", 8);

FileDecryptor::FileDecryptor()
{
}

SecByteBlock FileDecryptor::deriveKey(const QString &password,
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

bool FileDecryptor::hasEncryptionSignature(const QString &filePath)
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


FileResult FileDecryptor::decryptFile(const QString &filePath, const QString &password)
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

    if (isProtectedSystemPath(fileInfo)) {
        result.skipped = true;
        result.errorMessage = "System file is not allowed: " + filePath;
        return result;
    }

    if (!fileInfo.isReadable()) {
        result.errorMessage = "File is not readable: " + filePath;
        return result;
    }

    if (!fileInfo.isWritable()) {
        result.errorMessage = "File is not writable: " + filePath;
        return result;
    }

    QFileInfo dirInfo(fileInfo.absolutePath());

    if (isProtectedSystemPath(dirInfo)) {
        result.skipped = true;
        result.errorMessage = "Target system directory is not allowed: " + fileInfo.absolutePath();
        return result;
    }

    if (!dirInfo.isWritable()) {
        result.errorMessage = "Target directory is not writable: " + fileInfo.absolutePath();
        return result;
    }

    if (!hasEncryptionSignature(filePath)) {
        result.skipped = true;
        result.errorMessage = "File is not encrypted.";
        return result;
    }

    try {
        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            result.errorMessage = "Failed to open encrypted file: " + filePath;
            return result;
        }

        const qint64 minSize = ENCRYPTION_SIGNATURE.size() + SALT_SIZE + IV_SIZE + TAG_SIZE;
        if (inputFile.size() < minSize) {
            inputFile.close();
            result.errorMessage = "Invalid encrypted file format.";
            return result;
        }

        const QByteArray signature = inputFile.read(ENCRYPTION_SIGNATURE.size());
        if (signature != ENCRYPTION_SIGNATURE) {
            inputFile.close();
            result.errorMessage = "Invalid file signature.";
            return result;
        }

        const QByteArray saltBytes = inputFile.read(SALT_SIZE);
        if (saltBytes.size() != SALT_SIZE) {
            inputFile.close();
            result.errorMessage = "Failed to read salt.";
            return result;
        }

        const QByteArray ivBytes = inputFile.read(IV_SIZE);
        if (ivBytes.size() != IV_SIZE) {
            inputFile.close();
            result.errorMessage = "Failed to read IV.";
            return result;
        }

        const QByteArray encryptedData = inputFile.readAll();
        inputFile.close();

        if (encryptedData.size() < TAG_SIZE) {
            result.errorMessage = "Encrypted payload is too small.";
            return result;
        }

        SecByteBlock salt(reinterpret_cast<const byte*>(saltBytes.constData()), SALT_SIZE);
        SecByteBlock iv(reinterpret_cast<const byte*>(ivBytes.constData()), IV_SIZE);
        SecByteBlock key = deriveKey(password, salt, AES::MAX_KEYLENGTH);

        GCM<AES>::Decryption decryption;
        decryption.SetKeyWithIV(key, key.size(), iv, iv.size());

        std::string plainText;
        AuthenticatedDecryptionFilter adf(
            decryption,
            new StringSink(plainText),
            AuthenticatedDecryptionFilter::THROW_EXCEPTION,
            TAG_SIZE
            );

        adf.Put(reinterpret_cast<const byte*>(encryptedData.constData()),
                static_cast<size_t>(encryptedData.size()));
        adf.MessageEnd();

        QSaveFile outputFile(filePath);
        if (!outputFile.open(QIODevice::WriteOnly)) {
            result.errorMessage = "Failed to open output file for atomic write: " + filePath;
            return result;
        }

        if (!plainText.empty()) {
            const qint64 written = outputFile.write(plainText.data(),
                                                    static_cast<qint64>(plainText.size()));
            if (written != static_cast<qint64>(plainText.size())) {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write decrypted data.";
                return result;
            }
        }

        if (!outputFile.commit()) {
            result.errorMessage = "Failed to replace encrypted file atomically.";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainText.size();
        return result;
    }
    catch (const HashVerificationFilter::HashVerificationFailed &) {
        result.errorMessage = "Invalid password or corrupted encrypted file.";
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
