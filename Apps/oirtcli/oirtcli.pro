QT += core network
QT -= gui

TARGET = oirtcli
VERSION = 1.1.0.2

CONFIG += c++11 console
CONFIG -= app_bundle

DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERSION=\\\"$${VERSION}\\\" \
            APP_DESIGNER=\\\"Alex.A.Taranov\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
        main.cpp \
        qoirtcli.cpp \
        $${PWD}/../Shared/oirttask.cpp

HEADERS += \
        qoirtcli.h \
        $${PWD}/../Shared/oirttask.h

INCLUDEPATH += $${PWD}/../Shared

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
