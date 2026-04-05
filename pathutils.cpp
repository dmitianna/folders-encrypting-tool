#include "pathutils.h"
#include "windowsutils.h"
#include "unixutils.h"

bool isProtectedSystemPath(const QFileInfo& info)
{
    const QString absolutePath = info.absoluteFilePath();
    return hasWindowsSystemAttribute(absolutePath) || isUnixSystemPath(absolutePath);
}
