#include <QCoreApplication>
#include <QTextStream>
#include <QFileInfo>

//#include "../cryptomanager.h"
#include "../fileencryptor.h"
#include "../filedecryptor.h"
#include "../batchresult.h"
#include "../scanresult.h"
QTextStream in(stdin);
QTextStream out(stdout);
QTextStream err(stderr);

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

void printResult(const QString& testName, const ScanResult& result)
{
    QTextStream out(stdout);

    out << "==== " << testName << " ====\n";
    out << "success: " << (result.success ? "true" : "false") << "\n";

    if (!result.errorMessage.isEmpty()) {
        out << "error: " << result.errorMessage << "\n";
    }

    out << "files count: " << result.items.size() << "\n";

    for (const auto& item : result.items) {
        out << "  filePath: " << item.filePath << "\n";
        out << "  fileName: " << item.fileName << "\n";
        out << "  relativePath: " << item.relativePath << "\n";
        out << "  size: " << item.size << "\n";
    }

    out << "\n";
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


void writeFile(const QString& path, const QByteArray& data)
{
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(data);
        file.close();
    }
}

QByteArray readFile(const QString& path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return {};

    QByteArray data = file.readAll();
    file.close();
    return data;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    //filecrawler
    //FileCrawler crawler;
    /*
    // 1.пустой путь
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 1: empty path", result);
    }
    */

    // 2.Невалидный путь
    /*
    {
    QString path = ":::////invalid_path";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 2: invalid path", result);
    }
    */

    // 3.Путь с кавычками
    /*
    {
    QString path = "\""";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 3: quoted path", result);
    }
*/

    // 4.Несуществующая папка
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 4: folder does not exist", result);
    }
    */

    // 5.файл вместо папки
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 5: path is file, not folder", result);
    }
    */

    // 5* архив вместо папки
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 5: path is file, not folder", result);
    }
    */
    // 6. Пустая папка
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 6: empty folder", result);
    }
*/
    // 7. Обычная папка с файлами
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 7: valid folder", result);
    }
*/

    // 8. с вложенными каталогами
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 8: nested folders", result);
    }
    */
    // 9. Скрытые файлы
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 9: hidden files", result);
    }
*/
    // 10. Символьные ссылки
    /*
    {
    QString path = "";
    ScanResult result = crawler.scanFolder(path);
    printResult("Test 10: symlink handling", result);
    }
*/
//---------------------------------------------------------------------------
    FileEncryptor encryptor;
    FileDecryptor decryptor;

    // 0. Пустой путь при шифровании
/*
{
    QString path = "";
    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 1. Пустой пароль при шифровании
/*
{
    QString path = "";
    FileResult result = encryptor.encryptFile(path, "");
    printFileResult(result);
}
*/

    // Системный файл
    /*
{
    QString path = "";
    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);
}
*/
    // 2. Несуществующий файл при шифровании
    /*
{
    QString path = "";
    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 3. Папка вместо файла при шифровании
/*
{
    QString path = "";
    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 4.  шифрование обычного файла
/*
{
    QString path = "";
    QByteArray original = "Тестовый файл";

    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);

    QByteArray encrypted = readFile(path);

    if (encrypted != original)
        out << "content changed\n";
    else
        out << "content did not change\n";
}
*/

    // 5. Повторное шифрование уже зашифрованного файла
    /*
{
    QString path = "";
    encryptor.encryptFile(path, "1234");

    FileResult result = encryptor.encryptFile(path, "1234");
    printFileResult(result);
}
*/
    // 6. Шифрование и дешифрование пустого файла
/*
{
    QString path = "/testfolders/test3/empty.txt";

    FileResult enc = encryptor.encryptFile(path, "1234");
    printFileResult(enc);

    FileResult dec = decryptor.decryptFile(path, "1234");
    printFileResult(dec);

    QByteArray decrypted = readFile(path);
    if (decrypted.isEmpty())
        out << "[PASS] empty file restored correctly\n";
    else
        out << "[FAIL] empty file is not empty after decrypt\n";
}
*/

    // 7. Пустой путь при дешифровании
/*
{
    QString path = "";
    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 8. Пустой пароль при дешифровании
/*
{
    QString path = "/testfolders/test3/empty.txt";
    FileResult result = decryptor.decryptFile(path, "");
    printFileResult(result);
}
*/
    // 9. Несуществующий файл при дешифровании
/*
{
    QString path = "/testfiles/not_exist.txt";
    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 10. Папка вместо файла при дешифровании
/*
{
    QString path = "/testfolders/test3";
    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 11. Попытка дешифровать незашифрованный файл
    /*
{
    QString path = "/testfolders/test1/test1.txt";

    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 12. Успешное дешифрование и сравнение с оригиналом
    /*
{
    QString path = "/testfolders/test1/test1.txt";
    QByteArray original = "Hello encryption test 123";
    writeFile(path, original);

    FileResult enc = encryptor.encryptFile(path, "1234");
    printFileResult(enc);

    FileResult dec = decryptor.decryptFile(path, "1234");
    printFileResult(dec);

    QByteArray decrypted = readFile(path);

    if (decrypted == original)
        out << "[PASS] decrypted matches original\n";
    else
        out << "[FAIL] decrypted does not match original\n";
}
*/

    // 13. Дешифрование с неверным паролем
    /*
{
    QString path = "/testfolders/test1/test1.txt";
    writeFile(path, "Secret data");
    encryptor.encryptFile(path, "1234");

    FileResult result = decryptor.decryptFile(path, "wrong_password");
    printFileResult(result);
    decryptor.decryptFile(path, "1234");
}
*/
/*
    // 14. Повреждённый зашифрованный файл
{
    QString path = "/testfolders/test1/broken.txt";
    writeFile(path, "Secret data");
    encryptor.encryptFile(path, "1234");

    QByteArray data = readFile(path);
    if (!data.isEmpty()) {
        data[data.size() - 2] = 0x01;
        writeFile(path, data);
    }
    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/

    // 15. Слишком маленький файл с сигнатурой
/*
{
    QString path = "/testfolders/small_encrypted.bin";
    QByteArray bad = QByteArray::fromHex("DEADBEEFCAFEBABE");
    writeFile(path, bad);

    FileResult result = decryptor.decryptFile(path, "1234");
    printFileResult(result);
}
*/
    //-----------------------------------------------------------------------------------------------------------
//Cryptomanager
    //Проверка единственного экземпляра (Singleton)
    /*
    {
        CryptoManager& manager1 = CryptoManager::instance();
        CryptoManager& manager2 = CryptoManager::instance();

        if (&manager1 == &manager2)
            out << "[PASS] CryptoManager is singleton\n";
        else
            out << "[FAIL] CryptoManager is not singleton\n";
    }*/
    //Запрет создания объекта через конструктор
    //CryptoManager manager;

    //Запрет копирующего конструктора
    //CryptoManager copy = CryptoManager::instance();

    //Запрет оператора присваивания
    /*
    CryptoManager& manager1 = CryptoManager::instance();
    CryptoManager& manager2 = CryptoManager::instance();
    manager1 = manager2;
    */

    //-----------------------------

    // Шифрование папки с пустым паролем
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/test1";
    BatchResult result = manager.encryptFolder(folderPath, "");
    printBatchResult(result);
}
*/

    // Расшифровать папку с пустым паролем
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/test1";
    BatchResult result = manager.decryptFolder(folderPath, "");
    printBatchResult(result);
}
*/

    // Зашифровать папку с пустым путём
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "";
    BatchResult result = manager.encryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Поптыка расшифровать с пустым путём
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "";
    BatchResult result = manager.decryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Попытка шифрования для несуществующей папки
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/not_exist";
    BatchResult result = manager.encryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Попытка расшифровать для несуществующей папки
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/not_exist";
    BatchResult result = manager.decryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Шифрование пустой папки
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/empty";
    BatchResult result = manager.encryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Дешифрование пустой папки
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/empty";
    BatchResult result = manager.decryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // Шифрование папки с парой файлов, без вложенных папок
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    writeFile("/testfolders/test1/file1.txt", "Hello");
    writeFile("/testfolders/test1/file2.txt", "World");

    QString folderPath = "/testfolders/test1";
    BatchResult result = manager.encryptFolder(folderPath, "1234");
    printBatchResult(result);
}
*/

    // ДеШифрование папки с парой ранее зашифрованных файлов, без вложенных папок
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    manager.encryptFolder("/testfolders/test1", "1234");

    BatchResult result = manager.decryptFolder("/testfolders/test1", "1234");
    printBatchResult(result);
}
*/

    // Test 28: Повторное encryptFolder для уже зашифрованных файлов
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    writeFile("/testfolders/test1/file1.txt", "Hello");
    writeFile("/testfolders/test1/file2.txt", "World");

    manager.encryptFolder("/testfolders/test1", "1234");

    BatchResult result = manager.encryptFolder("/testfolders/test1", "1234");
    printBatchResult(result);
}
*/

    // Test 29: decryptFolder для обычных незашифрованных файлов
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    writeFile("/testfolders/test1/file1.txt", "Hello");
    writeFile("/testfolders/test1/file2.txt", "World");

    BatchResult result = manager.decryptFolder("/testfolders/test1", "1234");
    printBatchResult(result);
}
*/

    // Test 30: encryptFolder для вложенных каталогов
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    BatchResult result = manager.encryptFolder("/testfolders/nested", "1234");
    printBatchResult(result);
}
*/

    //decryptFolder для вложенных каталогов
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    manager.encryptFolder("/testfolders/nested", "1234");

    BatchResult result = manager.decryptFolder("/testfolders/nested", "1234");
    printBatchResult(result);
}
*/

    // Test 32: Полный цикл с проверкой восстановления содержимого
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QByteArray original1 = "Alpha";
    QByteArray original2 = "Beta";

    writeFile("/testfolders/test1/file1.txt", original1);
    writeFile("/testfolders/test1/file2.txt", original2);

    BatchResult enc = manager.encryptFolder("/testfolders/test1", "1234");
    printBatchResult(enc);

    BatchResult dec = manager.decryptFolder("/testfolders/test1", "1234");
    printBatchResult(dec);

    QByteArray file1 = readFile("/testfolders/test1/file1.txt");
    QByteArray file2 = readFile("/testfolders/test1/file2.txt");

    if (file1 == original1 && file2 == original2)
        out << "[PASS] CryptoManager folder roundtrip restored all files\n";
    else
        out << "[FAIL] CryptoManager folder roundtrip did not restore all files\n";
}
*/

    // Test 33: decryptFolder с неверным паролем
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    writeFile("/testfolders/test1/file1.txt", "Hello");
    writeFile("/testfolders/test1/file2.txt", "World");

    manager.encryptFolder("/testfolders/test1", "1234");

    BatchResult result = manager.decryptFolder("C:/testfolders/test1", "wrong_password");
    printBatchResult(result);
}
*/

    // Test 34: Повторное шифрование папки с добавлением нового файла и сменой пароля
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/test_mixed";

    writeFile(folderPath + "/file1.txt", "Hello");
    writeFile(folderPath + "/file2.txt", "World");

    BatchResult first = manager.encryptFolder(folderPath, "passwordA");
    printBatchResult(first);

    writeFile(folderPath + "/file3.txt", "New file");

    BatchResult second = manager.encryptFolder(folderPath, "passwordB");
    printBatchResult(second);
}
*/

    // Test 35: Дешифрование папки с файлами, зашифрованными разными паролями
    /*
{
    CryptoManager& manager = CryptoManager::instance();

    QString folderPath = "/testfolders/test_mixed";

    BatchResult result = manager.decryptFolder(folderPath, "passwordA");
    printBatchResult(result);
}
  */

    return 0;
}
