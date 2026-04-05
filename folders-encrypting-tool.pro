QT += core

CONFIG += c++17 cmdline

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        cryptomanager.cpp \
        filecrawler.cpp \
        filedecryptor.cpp \
        fileencryptor.cpp \
        main.cpp \
        pathutils.cpp \
        unixutils.cpp \
        windowsutils.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

HEADERS += \
    batchresult.h \
    cryptomanager.h \
    filecrawler.h \
    filedecryptor.h \
    fileencryptor.h \
    fileitem.h \
    fileresult.h \
    pathutils.h \
    scanresult.h \
    unixutils.h \
    windowsutils.h

INCLUDEPATH += $$PWD/third_party/cryptopp/include
LIBS += -L$$PWD/third_party/cryptopp/lib -lcryptopp

win32: LIBS += -lws2_32 -lgdi32 -lcrypt32
