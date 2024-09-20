INCLUDEPATH += $$PWD
DEPENDPATH  += $$PWD


HEADERS += \
    $$PWD/CYamlParser.h

SOURCES += \
    $$PWD/CYamlParser.cpp

win32{

INCLUDEPATH += $$PWD/lib/win
INCLUDEPATH += \
        $$PWD/  \
        $$PWD/include   \
        $$PWD/include/yaml  \

DEPENDPATH  += \
        $$PWD/  \
        $$PWD/include   \
        $$PWD/include/yaml  \

    debug{
        LIBS += -L$$PWD/lib/win/ -lyaml-cppd
    }
    release{
        LIBS += -L$$PWD/lib/win/ -lyaml-cpp
    }
}
unix{

    LIBS += -lyaml-cpp
}		


