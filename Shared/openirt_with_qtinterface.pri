include($${PWD}/../Sources/Basic/imagerecognizer.pri)

include($${PWD}/../Sources/ImageNet/imagenet.pri)
include($${PWD}/../Sources/Face/dlibfacedscr.pri)

SOURCES += $${PWD}/../Sources/Face/Qt/qfaceantispoofer.cpp \
           $${PWD}/../Sources/Face/Qt/qfacerecognizer.cpp

HEADERS += $${PWD}/../Sources/Face/Qt/qfaceantispoofer.h \
           $${PWD}/../Sources/Face/Qt/qfacerecognizer.h

INCLUDEPATH += $${PWD}/../Sources/Face/Qt
