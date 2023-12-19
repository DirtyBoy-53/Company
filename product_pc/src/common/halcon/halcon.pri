INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD
DEPENDPATH += $$PWD/lib

include($$PWD/include/include.pri)

LIBS += -L$$PWD/lib -lhalconcpp
