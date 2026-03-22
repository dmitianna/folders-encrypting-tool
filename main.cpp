#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>

#include "cryptomanager.h"

QTextStream in(stdin);
QTextStream out(stdout);
QTextStream err(stderr);

void printHelp()
{
    out << "Utility for encryption and decryption of files and folders\n";
    out << "Available commands:\n";
    out << "  encrypt  - encrypt file or folder\n";
    out << "  decrypt  - decrypt file or folder\n";
    out << "  exit     - exit program\n\n";
}

QString readNonEmptyLine(const QString& prompt)
{
    while (true)
    {
        out << prompt;
        out.flush();

        QString value = in.readLine().trimmed();

        if (!value.isEmpty())
            return value;

        err << "Input must not be empty.\n";
    }
}

void printFileResult(const FileResult& result)
{
    if (result.success)
        out << "Success: true\n";
    else
        out << "Success: false\n";

    if (result.skipped)
        out << "Skipped: true\n";
    else
        out << "Skipped: false\n";

    out << "Bytes processed: " << result.bytesProcessed << "\n";

    if (!result.errorMessage.isEmpty())
        out << "Message: " << result.errorMessage << "\n";
}

void printBatchResult(const BatchResult& result)
{
    if (result.success)
        out << "Success: true\n";
    else
        out << "Success: false\n";

    out << "Total files: " << result.totalFiles << "\n";
    out << "Processed: " << result.processedFiles << "\n";
    out << "Skipped: " << result.skippedFiles << "\n";
    out << "Failed: " << result.failedFiles << "\n";
    out << "Bytes processed: " << result.totalBytesProcessed << "\n";

    if (!result.errors.isEmpty())
    {
        out << "Errors:\n";
        for (int i = 0; i < result.errors.size(); ++i)
            out << "  - " << result.errors[i] << "\n";
    }
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    printHelp();

    while (true)
    {
        QString command = readNonEmptyLine("Enter command (encrypt/decrypt/exit): ");
        command = command.toLower();

        if (command == "exit")
        {
            out << "Program finished.\n";
            return 0;
        }

        if (command != "encrypt" && command != "decrypt")
        {
            err << "Unknown command. Please enter encrypt, decrypt or exit.\n\n";
            continue;
        }

        QString path = readNonEmptyLine("Enter file or folder path: ");
        QString password = readNonEmptyLine("Enter password: ");

        QFileInfo info(path);

        if (!info.exists())
        {
            err << "Error: path does not exist: " << path << "\n\n";
            continue;
        }

        CryptoManager& manager = CryptoManager::instance();

        if (info.isFile())
        {
            FileResult result;

            if (command == "encrypt")
                result = manager.encryptFile(path, password);
            else
                result = manager.decryptFile(path, password);

            printFileResult(result);
            out << "\n";
            continue;
        }

        if (info.isDir())
        {
            BatchResult result;

            if (command == "encrypt")
                result = manager.encryptFolder(path, password);
            else
                result = manager.decryptFolder(path, password);

            printBatchResult(result);
            out << "\n";
            continue;
        }

        err << "Error: specified path is neither a file nor a folder.\n\n";
    }

    return 0;
}
