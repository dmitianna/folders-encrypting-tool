#include "cryptomanager.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>

#include <files.h>
#include <filters.h>
#include <gcm.h>
#include <osrng.h>
#include <sha.h>
#include <pwdbased.h>

#include "scanresult.h"
#include "fileitem.h"
#include "pathutils.h"

using namespace CryptoPP;
const int MAX_PASSWORD_LENGTH = 64;
const QByteArray CryptoManager::ENCRYPTION_SIGNATURE("\xDE\xAD\xBE\xEF\xCA\xFE\xBA\xBE", 8);

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
//-------------------------------------------
bool CryptoManager::isPasswordValid(const QString& password, QString& errorMessage) const
{
    if (password.trimmed().isEmpty())
    {
        errorMessage = "Password must not be empty.";
        return false;
    }

    if (password.length() > MAX_PASSWORD_LENGTH)
    {
        errorMessage = "Password's length can not be more than 64 characters.";
        return false;
    }

    return true;
}
//-------------------------------------------
SecByteBlock CryptoManager::generateSalt(size_t size) const
{
    AutoSeededRandomPool rng;
    SecByteBlock salt(size);
    rng.GenerateBlock(salt, salt.size());
    return salt;
}
SecByteBlock CryptoManager::generateIV(size_t size) const
{
    AutoSeededRandomPool rng;
    SecByteBlock iv(size);
    rng.GenerateBlock(iv, iv.size());
    return iv;
}
SecByteBlock CryptoManager::deriveKey(const QString& password,const SecByteBlock& salt,size_t keySize)
const
{
    SecByteBlock key(keySize);
    const std::string pwd = password.toStdString();
    PKCS5_PBKDF2_HMAC<SHA256> pbkdf;
    pbkdf.DeriveKey(key, key.size(),0,reinterpret_cast<const byte*>(pwd.data()), pwd.size(),salt, salt.size(),100000);
    return key;
}

bool CryptoManager::hasEncryptionSignature(const QString& filePath) const
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly))
        return false;

    if (file.size() < ENCRYPTION_SIGNATURE.size())
    {
        file.close();
        return false;
    }
    const QByteArray signature = file.read(ENCRYPTION_SIGNATURE.size());
    file.close();
    return signature == ENCRYPTION_SIGNATURE;
}

//-------------------------------------------
FileResult CryptoManager::encryptFile(const QString& path, const QString& password)
{
    FileResult result;
    if (path.trimmed().isEmpty()){
        result.success = false;
        result.errorMessage = "Path must not be empty.";
        return result;
    }

    QString passwordError;
    if (!isPasswordValid(password, passwordError))
    {
        result.errorMessage = passwordError;
        return result;
    }
    //-------- для файлов -----------------------------------
    QFileInfo fileInfo(path);
    if (!fileInfo.exists())
    {
        result.errorMessage = "File does not exist: " + path;
        return result;
    }

    if (!fileInfo.isFile())
    {
        result.errorMessage = "Path is not a regular file: " + path;
        return result;
    }

    if (isProtectedSystemPath(fileInfo))
    {
        result.skipped = true;
        result.errorMessage = "System file is not allowed: " + path;
        return result;
    }

    if (!fileInfo.isReadable())
    {
        result.errorMessage = "File is not readable: " + path;
        return result;
    }
    //---------для директорий------------------------------------------
    QFileInfo dirInfo(fileInfo.absolutePath());

    if (isProtectedSystemPath(dirInfo))
    {
        result.skipped = true;
        result.errorMessage = "Target system directory is not allowed: " + fileInfo.absolutePath();
        return result;
    }

    if (!dirInfo.isWritable())
    {
        result.errorMessage = "Target directory is not writable: " + fileInfo.absolutePath();
        return result;
    }

    if (!fileInfo.isWritable())
    {
        result.errorMessage = "File is not writable: " + path;
        return result;
    }

    if (hasEncryptionSignature(path))
    {
        result.skipped = true;
        result.errorMessage = "File is already encrypted.";
        return result;
    }

    //---------шифрование------------------------------------
    try
    {
        QFile inputFile(path);
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            result.errorMessage = "Failed to open input file: " + path;
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

        if (!plainData.isEmpty())
        {
            aef.Put(reinterpret_cast<const byte*>(plainData.constData()),
                    static_cast<size_t>(plainData.size()));
        }
        aef.MessageEnd();

        QSaveFile outputFile(path);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            result.errorMessage = "Failed to open output file for write: " + path;
            return result;
        }

        if (outputFile.write(ENCRYPTION_SIGNATURE) != ENCRYPTION_SIGNATURE.size())
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write file signature.";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(salt.data()), SALT_SIZE) != SALT_SIZE)
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write salt.";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE) != IV_SIZE)
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write IV.";
            return result;
        }

        if (!encryptedData.empty())
        {
            const qint64 written = outputFile.write(encryptedData.data(),
                                                    static_cast<qint64>(encryptedData.size()));
            if (written != static_cast<qint64>(encryptedData.size()))
            {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write encrypted data.";
                return result;
            }
        }

        if (!outputFile.commit())
        {
            result.errorMessage = "Failed to replace original file.";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainData.size();
        return result;
    }
    catch (const Exception& e)
    {
        result.errorMessage = "Crypto++ error: " + QString::fromStdString(e.what());
        return result;
    }
    catch (const std::exception& e)
    {
        result.errorMessage = "Error: " + QString::fromStdString(e.what());
        return result;
    }
}

FileResult CryptoManager::decryptFile(const QString& path, const QString& password)
{
    FileResult result;
    if (path.trimmed().isEmpty())
    {
        result.success = false;
        result.errorMessage = "Path must not be empty.";
        return result;
    }

    QString passwordError;
    if (!isPasswordValid(password, passwordError))
    {
        result.errorMessage = passwordError;
        return result;
    }
    //-------------------------------------
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        result.errorMessage = "File does not exist: " + path;
        return result;
    }

    if (!fileInfo.isFile())
    {
        result.errorMessage = "Path is not a regular file: " + path;
        return result;
    }

    if (isProtectedSystemPath(fileInfo))
    {
        result.skipped = true;
        result.errorMessage = "System file is not allowed: " + path;
        return result;
    }

    if (!fileInfo.isReadable())
    {
        result.errorMessage = "File is not readable: " + path;
        return result;
    }

    if (!fileInfo.isWritable())
    {
        result.errorMessage = "File is not writable: " + path;
        return result;
    }
    //-----------------------------------------------

    QFileInfo dirInfo(fileInfo.absolutePath());

    if (isProtectedSystemPath(dirInfo))
    {
        result.skipped = true;
        result.errorMessage = "Target system directory is not allowed: " + fileInfo.absolutePath();
        return result;
    }

    if (!dirInfo.isWritable())
    {
        result.errorMessage = "Target directory is not writable: " + fileInfo.absolutePath();
        return result;
    }

    if (!hasEncryptionSignature(path))
    {
        result.skipped = true;
        result.errorMessage = "File is not encrypted.";
        return result;
    }
    //-----------------------------------------------
    try
    {
        QFile inputFile(path);
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            result.errorMessage = "Failed to open encrypted file: " + path;
            return result;
        }

        const qint64 minSize = ENCRYPTION_SIGNATURE.size() + SALT_SIZE + IV_SIZE + TAG_SIZE;
        if (inputFile.size() < minSize)
        {
            inputFile.close();
            result.errorMessage = "Invalid encrypted file format.";
            return result;
        }

        const QByteArray signature = inputFile.read(ENCRYPTION_SIGNATURE.size());
        if (signature != ENCRYPTION_SIGNATURE)
        {
            inputFile.close();
            result.errorMessage = "Invalid file signature.";
            return result;
        }

        const QByteArray saltBytes = inputFile.read(SALT_SIZE);
        if (saltBytes.size() != SALT_SIZE)
        {
            inputFile.close();
            result.errorMessage = "Failed to read salt.";
            return result;
        }

        const QByteArray ivBytes = inputFile.read(IV_SIZE);
        if (ivBytes.size() != IV_SIZE)
        {
            inputFile.close();
            result.errorMessage = "Failed to read IV.";
            return result;
        }

        const QByteArray encryptedData = inputFile.readAll();
        inputFile.close();

        if (encryptedData.size() < TAG_SIZE)
        {
            result.errorMessage = "Encrypted payload is too small.";
            return result;
        }

        SecByteBlock salt(reinterpret_cast<const byte*>(saltBytes.constData()), SALT_SIZE);
        SecByteBlock iv(reinterpret_cast<const byte*>(ivBytes.constData()), IV_SIZE);
        SecByteBlock key = deriveKey(password, salt, AES::MAX_KEYLENGTH);

        GCM<AES>::Decryption decryption;
        decryption.SetKeyWithIV(key, key.size(), iv, iv.size());

        std::string plainText;
        AuthenticatedDecryptionFilter adf(decryption,new StringSink(plainText),AuthenticatedDecryptionFilter::THROW_EXCEPTION,TAG_SIZE);
        adf.Put(reinterpret_cast<const byte*>(encryptedData.constData()),
                static_cast<size_t>(encryptedData.size()));
        adf.MessageEnd();

        QSaveFile outputFile(path);
        if (!outputFile.open(QIODevice::WriteOnly))
        {
            result.errorMessage = "Failed to open output file for atomic write: " + path;
            return result;
        }

        if (!plainText.empty())
        {
            const qint64 written = outputFile.write(plainText.data(),
                                                    static_cast<qint64>(plainText.size()));
            if (written != static_cast<qint64>(plainText.size()))
            {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write decrypted data.";
                return result;
            }
        }

        if (!outputFile.commit())
        {
            result.errorMessage = "Failed to replace encrypted file atomically.";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainText.size();
        return result;
    }
    catch (const HashVerificationFilter::HashVerificationFailed&)
    {
        result.errorMessage = "Invalid password or corrupted encrypted file.";
        return result;
    }
    catch (const Exception& e)
    {
        result.errorMessage = "Crypto++ error: " + QString::fromStdString(e.what());
        return result;
    }
    catch (const std::exception& e)
    {
        result.errorMessage = "Error: " + QString::fromStdString(e.what());
        return result;
    }
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
    QString passwordError;

    if (!isPasswordValid(password, passwordError))
    {
        batchResult.success = false;
        batchResult.errors.append(passwordError);
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
