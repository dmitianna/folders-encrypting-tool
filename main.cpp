#include <QCoreApplication>
#include <QDebug>
#include "filecrawler.h"

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "=== FILECRAWLER ===\n";

    // Путь для теста
    QString testPath = "testpath";
    if (argc > 1) {
        testPath = argv[1];
    }

    qDebug() << "Scanning:" << testPath << "\n";

    // Создаем и запускаем
    FileCrawler crawler;
    FileCrawler::ScanResult result = crawler.scanFolder(testPath);

    // Проверяем ошибки
    if (!result.success)
    {
        qDebug() << "ERROR:" << result.errorMessage;
        return 1;
    }

    // ПРОСТО ВЫВОДИМ СПИСОК
    qDebug() << "Found:" << result.items.size() << "\n";

    for (const auto &item : qAsConst(result.items)) {
        QString type = item.isDir ? "[DIR] " : "[FILE]";
        QString hidden = item.isHidden ? " (hidden)" : "";
        qDebug() << type + item.relativePath + hidden;
    }

    return 0;
}
