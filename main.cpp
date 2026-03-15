#include <QCoreApplication>
#include <QDebug>
#include "cryptomanager.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QString mode = "decrypt";   // change to "decrypt" to test decryption
    QString folderPath = "testpath";
    QString password = "test123";

    qDebug() << "Mode:" << mode;
    qDebug() << "Folder:" << folderPath;

    BatchResult result;

    if (mode == "encrypt")
    {
        qDebug() << "Encrypting folder...";
        result = CryptoManager::instance().encryptFolder(folderPath, password);
    }
    else if (mode == "decrypt")
    {
        qDebug() << "Decrypting folder...";
        result = CryptoManager::instance().decryptFolder(folderPath, password);
    }
    else
    {
        qDebug() << "Invalid mode.";
        return 1;
    }

    qDebug() << "Success:" << result.success;
    qDebug() << "Total files:" << result.totalFiles;
    qDebug() << "Processed:" << result.processedFiles;
    qDebug() << "Skipped:" << result.skippedFiles;
    qDebug() << "Failed:" << result.failedFiles;
    qDebug() << "Bytes processed:" << result.totalBytesProcessed;

    if (!result.errors.isEmpty())
    {
        qDebug() << "Errors:";
        for (int i = 0; i < result.errors.size(); ++i)
            qDebug() << result.errors[i];
    }

    return result.success ? 0 : 1;
}
