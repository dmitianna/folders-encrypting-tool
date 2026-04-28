#include "cryptomanager.h"

#include <QFile>
#include <QFileInfo>
#include <QSaveFile>
#include <QDir>
#include <QDirIterator>

#include <files.h>
#include <filters.h>
#include <gcm.h>
#include <osrng.h>
#include <sha.h>
#include <pwdbased.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

using namespace CryptoPP;

namespace
{
#ifdef Q_OS_WIN

bool hasWindowsSystemAttribute(const QString& absolutePath)
{
    const std::wstring nativePath = absolutePath.toStdWString();
    DWORD attrs = GetFileAttributesW(nativePath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return (attrs & FILE_ATTRIBUTE_SYSTEM) != 0;
}

bool isUnixSystemPath(const QString&)
{
    return false;
}

#elif defined(Q_OS_UNIX)

bool hasWindowsSystemAttribute(const QString&)
{
    return false;
}

bool isUnixSystemPath(const QString& absolutePath)
{
    return absolutePath == "/bin"   || absolutePath.startsWith("/bin/")   ||
           absolutePath == "/boot"  || absolutePath.startsWith("/boot/")  ||
           absolutePath == "/dev"   || absolutePath.startsWith("/dev/")   ||
           absolutePath == "/etc"   || absolutePath.startsWith("/etc/")   ||
           absolutePath == "/lib"   || absolutePath.startsWith("/lib/")   ||
           absolutePath == "/lib64" || absolutePath.startsWith("/lib64/") ||
           absolutePath == "/proc"  || absolutePath.startsWith("/proc/")  ||
           absolutePath == "/root"  || absolutePath.startsWith("/root/")  ||
           absolutePath == "/run"   || absolutePath.startsWith("/run/")   ||
           absolutePath == "/sbin"  || absolutePath.startsWith("/sbin/")  ||
           absolutePath == "/sys"   || absolutePath.startsWith("/sys/")   ||
           absolutePath == "/usr"   || absolutePath.startsWith("/usr/")   ||
           absolutePath == "/var"   || absolutePath.startsWith("/var/");
}

#else
bool hasWindowsSystemAttribute(const QString&)
{
    return false;
}

bool isUnixSystemPath(const QString&)
{
    return false;
}

#endif

bool isProtectedSystemPath(const QFileInfo& info)
{
    const QString absolutePath = info.absoluteFilePath();
    return hasWindowsSystemAttribute(absolutePath) || isUnixSystemPath(absolutePath);
}
}

const QByteArray CryptoManager::ENCRYPTION_SIGNATURE("\xDE\xAD\xBE\xEF\xCA\xFE\xBA\xBE", 8);

CryptoManager& CryptoManager::instance()
{
    static CryptoManager instance;
    return instance;
}

bool CryptoManager::isPasswordValid(const QString& password, QString& errorMessage) const
{
    if (password.trimmed().isEmpty())
    {
        errorMessage = "Password must not be empty";
        return false;
    }

    if (password.length() > MAX_PASSWORD_LENGTH)
    {
        errorMessage = "Password's length can not be more than 64 characters";
        return false;
    }

    return true;
}

bool CryptoManager::validateFileForProcessing(const QString& path, QString& errorMessage) const
{
    QFileInfo fileInfo(path);

    if (!fileInfo.exists())
    {
        errorMessage = "File does not exist";
        return false;
    }

    if (!fileInfo.isFile())
    {
        errorMessage = "Path is not a regular file";
        return false;
    }

    if (!fileInfo.isReadable())
    {
        errorMessage = "File is not readable";
        return false;
    }

    QFileInfo dirInfo(fileInfo.absolutePath());

    if (!dirInfo.isWritable())
    {
        errorMessage = "Target directory is not writable";
        return false;
    }

    if (!fileInfo.isWritable())
    {
        errorMessage = "File is not writable";
        return false;
    }

    return true;
}

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

FileResult CryptoManager::encryptFile(const QString& path, const QString& password)
{
    FileResult result;

    QString errorMessage;
    if (!validateFileForProcessing(path, errorMessage))
    {
        result.errorMessage = errorMessage;
        return result;
    }

    if (hasEncryptionSignature(path))
    {
        result.skipped = true;
        result.errorMessage = "File is already encrypted";
        return result;
    }

    try
    {
        QFile inputFile(path);
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            result.errorMessage = "Failed to open input file";
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
            result.errorMessage = "Failed to open output file for write";
            return result;
        }

        if (outputFile.write(ENCRYPTION_SIGNATURE) != ENCRYPTION_SIGNATURE.size())
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write file signature";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(salt.data()), SALT_SIZE) != SALT_SIZE)
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write salt";
            return result;
        }

        if (outputFile.write(reinterpret_cast<const char*>(iv.data()), IV_SIZE) != IV_SIZE)
        {
            outputFile.cancelWriting();
            result.errorMessage = "Failed to write IV";
            return result;
        }

        if (!encryptedData.empty())
        {
            const qint64 written = outputFile.write(encryptedData.data(),
                                                    static_cast<qint64>(encryptedData.size()));
            if (written != static_cast<qint64>(encryptedData.size()))
            {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write encrypted data";
                return result;
            }
        }

        if (!outputFile.commit())
        {
            result.errorMessage = "Failed to replace original file";
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
    QString errorMessage;
    if (!validateFileForProcessing(path, errorMessage))
    {
        result.errorMessage = errorMessage;
        return result;
    }

    if (!hasEncryptionSignature(path))
    {
        result.skipped = true;
        result.errorMessage = "File is not encrypted";
        return result;
    }
    try
    {
        QFile inputFile(path);
        if (!inputFile.open(QIODevice::ReadOnly))
        {
            result.errorMessage = "Failed to open encrypted file";
            return result;
        }

        const qint64 minSize = ENCRYPTION_SIGNATURE.size() + SALT_SIZE + IV_SIZE + TAG_SIZE;
        if (inputFile.size() < minSize)
        {
            inputFile.close();
            result.errorMessage = "Invalid encrypted file format";
            return result;
        }

        const QByteArray signature = inputFile.read(ENCRYPTION_SIGNATURE.size());
        if (signature != ENCRYPTION_SIGNATURE)
        {
            inputFile.close();
            result.errorMessage = "Invalid file signature";
            return result;
        }

        const QByteArray saltBytes = inputFile.read(SALT_SIZE);
        if (saltBytes.size() != SALT_SIZE)
        {
            inputFile.close();
            result.errorMessage = "Failed to read salt";
            return result;
        }

        const QByteArray ivBytes = inputFile.read(IV_SIZE);
        if (ivBytes.size() != IV_SIZE)
        {
            inputFile.close();
            result.errorMessage = "Failed to read IV";
            return result;
        }

        const QByteArray encryptedData = inputFile.readAll();
        inputFile.close();

        if (encryptedData.size() < TAG_SIZE)
        {
            result.errorMessage = "Encrypted payload is too small";
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
            result.errorMessage = "Failed to open output file for atomic write";
            return result;
        }

        if (!plainText.empty())
        {
            const qint64 written = outputFile.write(plainText.data(),
                                                    static_cast<qint64>(plainText.size()));
            if (written != static_cast<qint64>(plainText.size()))
            {
                outputFile.cancelWriting();
                result.errorMessage = "Failed to write decrypted data";
                return result;
            }
        }

        if (!outputFile.commit())
        {
            result.errorMessage = "Failed to replace encrypted file atomically";
            return result;
        }

        result.success = true;
        result.bytesProcessed = plainText.size();
        return result;
    }
    catch (const HashVerificationFilter::HashVerificationFailed&)
    {
        result.errorMessage = "Invalid password or corrupted encrypted file";
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

CryptoManager::ScanResult CryptoManager::scanFolder(const QString& path) const
{
    ScanResult result;
    result.success = false;

    QFileInfo dirInfo(path);
    if (!dirInfo.exists())
    {
        result.errorMessage = "Folder does not exist: " + path;
        return result;
    }

    if (!dirInfo.isDir())
    {
        result.errorMessage = "Path is not a folder: " + path;
        return result;
    }

    if (!dirInfo.isReadable())
    {
        result.errorMessage = "Folder is not readable: " + path;
        return result;
    }

    if (dirInfo.isHidden())
    {
        result.errorMessage = "Hidden folders are not allowed: " + path;
        return result;
    }

    if (isProtectedSystemPath(dirInfo))
    {
        result.errorMessage = "System folders are not allowed: " + path;
        return result;
    }


    QDirIterator it(path, QDir::Files | QDir::Hidden | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);

    while (it.hasNext())
    {
        it.next();
        QFileInfo entry = it.fileInfo();

        if (entry.isSymLink())
        {
            result.ignoredFiles++;
            result.ignoredMessages.append(entry.absoluteFilePath() + " : symbolic link");
            continue;
        }


        if (entry.isHidden())
        {
            result.ignoredFiles++;
            result.ignoredMessages.append(entry.absoluteFilePath() + " : hidden file");
            continue;
        }

        if (isProtectedSystemPath(entry))
        {
            result.ignoredFiles++;
            result.ignoredMessages.append(entry.absoluteFilePath() + " : system file");
            continue;
        }
        result.files.append(entry.absoluteFilePath());
    }

    result.success = true;
    return result;
}

BatchResult CryptoManager::processFolder(const QString& folderPath,const QString& password,bool shouldEncrypt)
{
    BatchResult batchResult;

    ScanResult scanResult = scanFolder(folderPath);

    if (!scanResult.success) {
        batchResult.success = false;
        batchResult.errors.append(scanResult.errorMessage);
        return batchResult;
    }

    QString passwordError;

    if (!isPasswordValid(password, passwordError))
    {
        batchResult.success = false;
        batchResult.errors.append(passwordError);
        return batchResult;
    }

    batchResult.totalFiles = scanResult.files.size() + scanResult.ignoredFiles;
    batchResult.ignoredFiles = scanResult.ignoredFiles;
    batchResult.ignoredMessages = scanResult.ignoredMessages;
    const QStringList& files = scanResult.files;


    for (const QString& filePath : files)
    {

        FileResult fileResult;

        if (shouldEncrypt)
        {
            fileResult = encryptFile(filePath, password);
        }
        else
        {
            fileResult = decryptFile(filePath, password);
        }

        if (fileResult.success)
        {
            batchResult.processedFiles++;
            batchResult.totalBytesProcessed += fileResult.bytesProcessed;
        }
        else if (fileResult.skipped)
        {
            batchResult.skippedFiles++;

            QString message = filePath;
            if (!fileResult.errorMessage.isEmpty())
                message += " : " + fileResult.errorMessage;

            batchResult.skippedMessages.append(message);
        }
        else
        {
            batchResult.failedFiles++;

            QString message = filePath;
            if (!fileResult.errorMessage.isEmpty())
                message += " : " + fileResult.errorMessage;

            batchResult.errors.append(message);
        }
    }

    batchResult.success = (batchResult.failedFiles == 0);
    return batchResult;
}
