#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>

#include "cryptomanager.h"

QTextStream in(stdin);
QTextStream out(stdout);

void printHelp()
{
    out << "Utility for encryption and decryption of folders\n";
    out << "Available commands:\n";
    out << "  encrypt  - encrypt folder\n";
    out << "  decrypt  - decrypt folder\n";
    out << "  exit     - exit program\n\n";

    out << "Tip: You can type 'exit' at any time to leave the program.\n";
    out << "Tip: If the password is lost, encrypted files cannot be recovered.\n\n";

    out << "Note: Due to technical limitations, hidden files and directories are not processed.\n";
    out << "      Contents of hidden directories are also excluded from processing.\n\n";
}

QString readNonEmptyLine(const QString& prompt)
{
    while (true)
    {
        out << prompt;
        out.flush();

        QString value = in.readLine().trimmed();

        QString lower = value.toLower();

        if (lower == "exit")
        {
            out << "Program finished.\n";
            out.flush();
            exit(0);
        }

        if ((value.startsWith('"') && value.endsWith('"')) ||
            (value.startsWith('\'') && value.endsWith('\'')))
        {
            value = value.mid(1, value.length() - 2).trimmed();
        }

        if (!value.isEmpty())
            return value;

        out << "Input must not be empty.\n";
        out.flush();
    }
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

    out.flush();
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
            out.flush();
            return 0;
        }

        if (command != "encrypt" && command != "decrypt")
        {
            out << "Unknown command. Please enter encrypt, decrypt or exit.\n\n";
            out.flush();
            continue;
        }

        QString path = readNonEmptyLine("Enter folder path: ");

        QFileInfo info(path);

        if (!info.exists())
        {
            out << "Error: path does not exist: " << path << "\n\n";
            out.flush();
            continue;
        }

        if (!info.isDir())
        {
            out << "Error: specified path is not a folder.\n\n";
            out.flush();
            continue;
        }

        QString password = readNonEmptyLine("Enter password: ");
        CryptoManager& manager = CryptoManager::instance();
        BatchResult result;

        if (command == "encrypt")
            result = manager.encryptFolder(path, password);
        else
            result = manager.decryptFolder(path, password);

        printBatchResult(result);
        out << "\n";
        out.flush();
    }

    return 0;
}
