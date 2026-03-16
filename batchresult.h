#ifndef BATCHRESULT_H
#define BATCHRESULT_H

#include <QStringList>

struct BatchResult
{
    bool success = false;
    int totalFiles = 0;
    int processedFiles = 0;
    int skippedFiles = 0;
    int failedFiles = 0;
    qint64 totalBytesProcessed = 0;
    QStringList errors;
};

#endif // BATCHRESULT_H
