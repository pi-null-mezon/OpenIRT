QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

TARGET = imgdirdscrgen
VERSION = 1.0.0.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\"

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp

include($${PWD}/../../Shared/opencv.pri)
include($${PWD}/../../Shared/dlib.pri)
include($${PWD}/../../Sources/Basic/imagerecognizer.pri)

#include($${PWD}/../../Sources/ImageNet/imagenet.pri)
#include($${PWD}/../../Sources/Face/dlibfacerecognizer.pri)
include($${PWD}/../../Sources/Kaggle/Whales/dlibwhalesrecognizer.pri)
#include($${PWD}/../../Sources/Kaggle/Furniture/furniturerecognizer.pri)

unix {
   target.path = /usr/local/bin
   INSTALLS += target
}
