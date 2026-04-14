QT += core
CONFIG += console c++17
CONFIG -= app_bundle

TEMPLATE = app
TARGET = encrypt_cli

SOURCES += \
    main.cpp \
    ../cryptomanager.cpp \
    ../filecrawler.cpp \
    ../pathutils.cpp \
    ../unixutils.cpp \
    ../windowsutils.cpp

HEADERS += \
    ../batchresult.h \
    ../cryptomanager.h \
    ../filecrawler.h \
    ../fileitem.h \
    ../fileresult.h \
    ../pathutils.h \
    ../scanresult.h \
    ../unixutils.h \
    ../windowsutils.h

INCLUDEPATH += $$PWD/../third_party/cryptopp/include
LIBS += -L$$PWD/../third_party/cryptopp/lib -lcryptopp

win32: LIBS += -lws2_32 -lgdi32 -lcrypt32
