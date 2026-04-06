#include "windowsutils.h"

#ifdef Q_OS_WIN
#include <windows.h>

bool hasWindowsSystemAttribute(const QString& absolutePath)
{
    const std::wstring nativePath = absolutePath.toStdWString();
    DWORD attrs = GetFileAttributesW(nativePath.c_str());

    if (attrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return (attrs & FILE_ATTRIBUTE_SYSTEM) != 0;
}

#else

bool hasWindowsSystemAttribute(const QString&)
{
    return false;
}

#endif
