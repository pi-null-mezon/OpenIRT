#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T12:06:10
#
#-------------------------------------------------

QT += core network
QT -= gui

CONFIG += c++11

TARGET = dialyzersrv
VERSION = 1.1.0.0

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERSION=\\\"$${VERSION}\\\" \
            APP_DESIGNER=\\\"Alex.A.Taranov\\\"

SOURCES += main.cpp

include($${PWD}/../../../Shared/opencv.pri)
include($${PWD}/../../../Shared/dlib.pri)
include($${PWD}/../../../Sources/Basic/imagerecognizer.pri)

SOURCES += $${PWD}/../../../Sources/Dialyzer/dialyzerrecognizer.cpp \
           $${PWD}/../../../Sources/Basic/Qt/qrecognizer.cpp \
           $${PWD}/../../Shared/qoirtserver.cpp \
           $${PWD}/../../Shared/oirttask.cpp

HEADERS += $${PWD}/../../../Sources/Dialyzer/dialyzerrecognizer.h \
           $${PWD}/../../../Sources/Basic/Qt/qrecognizer.h \
           $${PWD}/../../Shared/qoirtserver.h \
           $${PWD}/../../Shared/oirttask.h

INCLUDEPATH += $${PWD}/../../../Sources/Dialyzer \
               $${PWD}/../../../Sources/Basic/Qt \
               $${PWD}/../../Shared

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
