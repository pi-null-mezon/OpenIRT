SOURCES += $${PWD}/imagerecognizer.cpp \
           $${PWD}/predict_collector.cpp \
           $${PWD}/cnnimagerecognizer.cpp \
           $${PWD}/googlenetrecognizer.cpp \
           $${PWD}/resnet50imagenetrecognizer.cpp \
           $${PWD}/squeezenetimagenetrecognizer.cpp

HEADERS +=  $${PWD}/imagerecognizer.hpp \
            $${PWD}/precompiled.hpp \
            $${PWD}/imagerec_basic.hpp \
            $${PWD}/predict_collector.hpp \
            $${PWD}/cnnimagerecognizer.hpp \
            $${PWD}/googlenetrecognizer.h \
            $${PWD}/resnet50imagenetrecognizer.h \
            $${PWD}/squeezenetimagenetrecognizer.h

INCLUDEPATH += $${PWD}

include($${PWD}/../../Shared/opencv.pri)
