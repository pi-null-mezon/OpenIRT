#-------------------------------------------------
#
# Project created by QtCreator 2017-06-09T12:06:10
#
#-------------------------------------------------

QT += core network
QT -= gui

CONFIG += c++11

TARGET = oirtsrv
VERSION = 1.5.0.0

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
include(openfrt.pri)

SOURCES += $${PWD}/../../../Sources/Face/dlibfacerecognizer.cpp \
           $${PWD}/../../../Sources/Basic/Qt/qrecognizer.cpp \
           $${PWD}/../../Shared/qoirtserver.cpp \
           $${PWD}/../../Shared/oirttask.cpp

HEADERS += $${PWD}/../../../Sources/Face/dlibfacerecognizer.h \
           $${PWD}/../../../Sources/Basic/Qt/qrecognizer.h \
           $${PWD}/../../Shared/qoirtserver.h \
           $${PWD}/../../Shared/oirttask.h

INCLUDEPATH += $${PWD}/../../../Sources/Face \
               $${PWD}/../../../Sources/Basic/Qt \
               $${PWD}/../../Shared

CONFIG(release, debug|release): DEFINES += QT_NO_DEBUG_OUTPUT

usecnnfacedetector {
    DEFINES += FORCE_TO_USE_CNN_FACE_DETECTOR
    message('FORCE_TO_USE_CNN_FACE_DETECTOR' defined)
    message(Opencv\'s CNN face detector will be used)
} else {
    message(Dlib\'s face detector will be used)
}

unix {
    target.path = /usr/local/bin
    INSTALLS += target

    resources.path = /usr/local/bin
    resources.files = $${PWD}/resources/*
    INSTALLS += resources
}

