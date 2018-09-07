#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T12:06:10
#
#-------------------------------------------------

QT += core network
QT -= gui

CONFIG += c++11

TARGET = oirtsrv
VERSION = 1.0.5.0

TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle

DEFINES +=  APP_NAME=\\\"$${TARGET}\\\" \
            APP_VERSION=\\\"$${VERSION}\\\" \
            APP_DESIGNER=\\\"Alex.A.Taranov\\\"

SOURCES += main.cpp\
           qoirtserver.cpp \
           $${PWD}/shared/oirttask.cpp

HEADERS  += qoirtserver.h \
            $${PWD}/shared/oirttask.h

INCLUDEPATH += $${PWD}/shared

include($${PWD}/../../Shared/opencv.pri)
include($${PWD}/../../Shared/dlib.pri)
include($${PWD}/../../Sources/Basic/imagerecognizer.pri)

SOURCES += $${PWD}/../../Sources/Face/dlibfacerecognizer.cpp \
           $${PWD}/../../Sources/Face/Qt/qfacerecognizer.cpp

HEADERS += $${PWD}/../../Sources/Face/dlibfacerecognizer.h \
           $${PWD}/../../Sources/Face/Qt/qfacerecognizer.h

INCLUDEPATH += $${PWD}/../../Sources/Face \
               $${PWD}/../../Sources/Face/Qt

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT
