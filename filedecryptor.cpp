#include "filedecryptor.h"

#include <QFile>
#include <QFileInfo>

#include <files.h>
#include <filters.h>
#include <modes.h>

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

QString FileDecryptor::makeTempFilePath(const QString &filePath) const
{
    return filePath + ".tmp_dec";
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

    if (!fileInfo.isReadable()) {
        result.errorMessage = "File is not readable: " + filePath;
        return result;
    }

    if (!hasEncryptionSignature(filePath)) {
        result.skipped = true;
        result.errorMessage = "File is not encrypted.";
        return result;
    }

    const QString tempPath = makeTempFilePath(filePath);

    if (QFile::exists(tempPath)) {
        if (!QFile::remove(tempPath)) {
            result.errorMessage = "Temporary file already exists and cannot be removed: " + tempPath;
            return result;
        }
    }

    try {
        QFile inputFile(filePath);
        if (!inputFile.open(QIODevice::ReadOnly)) {
            result.errorMessage = "Failed to open encrypted file: " + filePath;
            return result;
        }

        const int headerSize = ENCRYPTION_SIGNATURE.size() + SALT_SIZE + IV_SIZE;
        if (inputFile.size() < headerSize) {
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

        const QByteArray cipherData = inputFile.readAll();
        inputFile.close();

        SecByteBlock salt(reinterpret_cast<const byte*>(saltBytes.constData()), SALT_SIZE);
        SecByteBlock iv(reinterpret_cast<const byte*>(ivBytes.constData()), IV_SIZE);
        SecByteBlock key = deriveKey(password, salt, AES::MAX_KEYLENGTH);

        CBC_Mode<AES>::Decryption decryption;
        decryption.SetKeyWithIV(key, key.size(), iv, iv.size());

        std::string plainText;
        StringSource(
            reinterpret_cast<const byte*>(cipherData.constData()),
            static_cast<size_t>(cipherData.size()),
            true,
            new StreamTransformationFilter(
                decryption,
                new StringSink(plainText)
                )
            );

        QFile tempFile(tempPath);
        if (!tempFile.open(QIODevice::WriteOnly)) {
            result.errorMessage = "Failed to create temporary file: " + tempPath;
            return result;
        }

        if (!plainText.empty()) {
            const qint64 written = tempFile.write(plainText.data(), static_cast<qint64>(plainText.size()));
            if (written != static_cast<qint64>(plainText.size())) {
                tempFile.close();
                QFile::remove(tempPath);
                result.errorMessage = "Failed to write decrypted data.";
                return result;
            }
        }

        tempFile.close();

        QFileInfo tempInfo(tempPath);
        if (!tempInfo.exists()) {
            QFile::remove(tempPath);
            result.errorMessage = "Temporary decrypted file was not created.";
            return result;
        }

        if (!QFile::remove(filePath)) {
            QFile::remove(tempPath);
            result.errorMessage = "Failed to remove encrypted file.";
            return result;
        }

        QFile renamedFile(tempPath);
        if (!renamedFile.rename(filePath)) {
            result.errorMessage = "Failed to replace encrypted file with decrypted file.";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainText.size();
        return result;
    }
    catch (const Exception &e) {
        QFile::remove(tempPath);
        result.errorMessage = "Crypto++ error: " + QString::fromStdString(e.what());
        return result;
    }
    catch (const std::exception &e) {
        QFile::remove(tempPath);
        result.errorMessage = "Error: " + QString::fromStdString(e.what());
        return result;
    }
}
