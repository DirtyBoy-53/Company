INCLUDEPATH += \
        $$PWD/  \
        $$PWD/include   \
        $$PWD/include/yaml-cpp  \
DEPENDPATH  += \
        $$PWD/  \
        $$PWD/include   \
        $$PWD/include/yaml-cpp  \

# HEADERS += \
#         $$PWD/include/yaml-cpp/yaml.h

HEADERS += \
    $$PWD/CYamlParser.h

SOURCES += \
    $$PWD/CYamlParser.cpp

INCLUDEPATH += $$PWD/lib/win
win32{
    debug{
            LIBS += -L$$PWD/lib/win/ -lyaml-cppd
    }
    release{
            LIBS += -L$$PWD/lib/win/ -lyaml-cpp
    }
}
unix{
    debug{

    }
    release{

    }
}		


