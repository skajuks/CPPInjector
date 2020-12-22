QT       += core gui widgets

CONFIG += c++17

# QMAKE_LFLAGS += /MANIFESTUAC:$$quote(\"level=\'requireAdministrator\' uiAccess=\'false\'\")
# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    qprocessinfo.cpp

HEADERS += \
    main.h \
    qprocessinfo.h

FORMS += \
    main.ui

RC_ICONS = icon.ico

