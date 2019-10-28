INCLUDEPATH += $${PWD} \
               $${PWD}/../Basic

HEADERS += \
    $${PWD}/docrecognizer.h \
    $${PWD}/../Basic/imageclassifier.h \
    $${PWD}/../Basic/cnnimageclassifier.h

SOURCES += \
    $${PWD}/docrecognizer.cpp \
    $${PWD}/../Basic/imageclassifier.cpp \
    $${PWD}/../Basic/cnnimageclassifier.cpp
