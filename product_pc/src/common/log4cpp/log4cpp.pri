INCLUDEPATH += $$PWD/include
DEPENDPATH += $$PWD/lib

include($$PWD/include/include.pri)

LIBS += -L$$PWD/lib -llog4cppLIB
