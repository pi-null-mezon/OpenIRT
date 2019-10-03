
TARGET = classifyimage
VERSION = 1.0.0.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A._Taranov\\\"

TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += main.cpp

include($${PWD}/../../Sources/Basic/imageclassifier.pri)
#include($${PWD}/../../Sources/Face/faceageclassifier.pri)
include($${PWD}/../../Sources/Face/spoofingattackdetector.pri)

include($${PWD}/../../Shared/opencv.pri)
include($${PWD}/../../Shared/dlib.pri)
