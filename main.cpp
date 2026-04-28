#include <QCoreApplication>
#include <QTextStream>

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
}

QString normalizeInput(QString value)
{
    value = value.trimmed();

    if (value.length() >= 2 &&
        ((value.startsWith('"') && value.endsWith('"')) ||
         (value.startsWith('\'') && value.endsWith('\''))))
    {
        value = value.mid(1, value.length() - 2).trimmed();
    }

    return value;
}

QString readLine(const QString& prompt)
{
    out << prompt;
    out.flush();

    return normalizeInput(in.readLine());
}

void printBatchResult(const BatchResult& result)
{
    out << "Success: " << (result.success ? "true" : "false") << "\n";
    out << "Total files: " << result.totalFiles << "\n";
    out << "Processed: " << result.processedFiles << "\n";
    out << "Skipped: " << result.skippedFiles << "\n";
    out << "Ignored: " << result.ignoredFiles << "\n";
    out << "Failed: " << result.failedFiles << "\n";
    out << "Bytes processed: " << result.totalBytesProcessed << "\n";

    if (!result.errors.isEmpty())
    {
        out << "Errors:\n";
        for (const QString& msg : result.errors)
            out << "  - " << msg << "\n";
    }

    if (!result.skippedMessages.isEmpty())
    {
        out << "Skipped:\n";
        for (const QString& msg : result.skippedMessages)
            out << "  - " << msg << "\n";
    }

    if (!result.ignoredMessages.isEmpty())
    {
        out << "Ignored:\n";
        for (const QString& msg : result.ignoredMessages)
            out << "  - " << msg << "\n";
    }

    out << "\n";
}
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    printHelp();
    while (true)
    {
        QString command = readLine("Enter command (encrypt/decrypt/exit): ").toLower();

        if (command == "exit")
        {
            out << "Program finished.\n";
            out.flush();
            return 0;
        }

        if (command.isEmpty())
        {
            out << "Input must not be empty.\n\n";
            out.flush();
            continue;
        }

        if (command != "encrypt" && command != "decrypt")
        {
            out << "Unknown command. Please enter encrypt, decrypt or exit.\n\n";
            out.flush();
            continue;
        }

        QString path = readLine("Enter folder path: ");

        if (path.compare("exit", Qt::CaseInsensitive) == 0)
        {
            out << "Program finished.\n";
            out.flush();
            return 0;
        }

        if (path.isEmpty())
        {
            out << "Input must not be empty. Operation canceled.\n\n";
            out.flush();
            continue;
        }

        QString password = readLine("Enter password: ");

        if (password.compare("exit", Qt::CaseInsensitive) == 0)
        {
            out << "Program finished.\n";
            out.flush();
            return 0;
        }

        if (password.isEmpty())
        {
            out << "Input must not be empty. Operation canceled.\n\n";
            out.flush();
            continue;
        }

        CryptoManager& manager = CryptoManager::instance();
        BatchResult result;

        if (command == "encrypt")
            result = manager.encryptFolder(path, password);
        else
            result = manager.decryptFolder(path, password);

        printBatchResult(result);
        out.flush();
    }

    return 0;
}
