#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T12:06:10
#
#-------------------------------------------------

QT += core network
QT -= gui

CONFIG += c++11

TARGET = antispoofingsrv
VERSION = 1.0.0.0

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERSION=\\\"$${VERSION}\\\" \
            APP_DESIGNER=\\\"Alex.A.Taranov\\\"

SOURCES += main.cpp

include($${PWD}/../../../Shared/opencv.pri)
include($${PWD}/../../../Shared/dlib.pri)
include($${PWD}/../../../Sources/Documents/docrecognizer.pri)

SOURCES += $${PWD}/../../../Sources/Basic/Qt/qclassifier.cpp \
           $${PWD}/../../Shared/qoictserver.cpp \
           $${PWD}/../../Shared/oicttask.cpp

HEADERS += $${PWD}/../../../Sources/Basic/Qt/qclassifier.h \
           $${PWD}/../../Shared/qoictserver.h \
           $${PWD}/../../Shared/oicttask.h

INCLUDEPATH += $${PWD}/../../../Sources/Basic/Qt \
               $${PWD}/../../Shared

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

unix {
    target.path = /usr/local/bin
    INSTALLS += target
}
