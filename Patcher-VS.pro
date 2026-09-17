QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += network

CONFIG += c++17
CONFIG += static

QMAKE_LFLAGS_WINDOWS += /MANIFESTUAC:"level='requireAdministrator'"

RC_ICONS = app.ico

# static OpenSSL, used for SHA-256; override with: qmake OPENSSL_DIR=<path>
isEmpty(OPENSSL_DIR): OPENSSL_DIR = C:/openssl/STATIC/x64/Release
INCLUDEPATH += $$OPENSSL_DIR/include
LIBS += -L$$OPENSSL_DIR/Lib -llibcrypto -llibssl

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    downloadManager.cpp \
    label_exit.cpp \
    label_minimize.cpp \
    firstThread.cpp \
    DownloadFile.cpp

HEADERS += \
    config.h \
    mainwindow.h \
    downloadManager.h \
    label_exit.h \
    label_minimize.h \
    firstThread.h \
    DownloadFile.h

RESOURCES += \
    Resource.qrc

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
