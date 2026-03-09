#ifndef FILERESULT_H
#define FILERESULT_H

#include <QString>

struct FileResult
{
    bool success = false;
    bool skipped = false;
    QString errorMessage;
    qint64 bytesProcessed = 0;
};

#endif
