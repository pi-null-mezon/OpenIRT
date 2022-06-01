QT += quick qml multimedia widgets

TEMPLATE=app

TARGET=OIRTClient
VERSION=1.0.0.0

DEFINES += APP_NAME=\\\"$${TARGET}\\\" \
           APP_VERSION=\\\"$${VERSION}\\\" \
           APP_DESIGNER=\\\"Alex_A._Taranov\\\"


SOURCES += \
           main.cpp \
           qprocessor.cpp \
           qrecognitiontaskposter.cpp

RESOURCES += declarative-camera.qrc

HEADERS += \
    qprocessor.h \
    qrecognitiontaskposter.h
