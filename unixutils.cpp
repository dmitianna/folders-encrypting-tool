#include "unixutils.h"

#ifndef Q_OS_WIN

bool isUnixSystemPath(const QString& absolutePath)
{
    return absolutePath == "/bin"   || absolutePath.startsWith("/bin/")   ||
           absolutePath == "/boot"  || absolutePath.startsWith("/boot/")  ||
           absolutePath == "/dev"   || absolutePath.startsWith("/dev/")   ||
           absolutePath == "/etc"   || absolutePath.startsWith("/etc/")   ||
           absolutePath == "/lib"   || absolutePath.startsWith("/lib/")   ||
           absolutePath == "/lib64" || absolutePath.startsWith("/lib64/") ||
           absolutePath == "/proc"  || absolutePath.startsWith("/proc/")  ||
           absolutePath == "/root"  || absolutePath.startsWith("/root/")  ||
           absolutePath == "/run"   || absolutePath.startsWith("/run/")   ||
           absolutePath == "/sbin"  || absolutePath.startsWith("/sbin/")  ||
           absolutePath == "/sys"   || absolutePath.startsWith("/sys/")   ||
           absolutePath == "/usr"   || absolutePath.startsWith("/usr/")   ||
           absolutePath == "/var"   || absolutePath.startsWith("/var/");
}

#else

bool isUnixSystemPath(const QString&)
{
    return false;
}

#endif
