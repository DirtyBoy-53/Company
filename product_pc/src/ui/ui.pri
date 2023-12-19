INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include ($$PWD/components/components.pri)
include ($$PWD/templates/templates.pri)

FORMS += \
    $$PWD/XyAlgorithmDebug.ui \
    $$PWD/XyMesDebug.ui \
    $$PWD/XyScanDialog.ui \
    $$PWD/loginwindow.ui \
    $$PWD/mainwindow.ui \
    $$PWD/selectorder.ui \
    $$PWD/csm_login.ui

HEADERS += \
    $$PWD/XyAlgorithmDebug.h \
    $$PWD/XyMesDebug.h \
    $$PWD/XyScanDialog.h \
    $$PWD/loginwindow.h \
    $$PWD/mainwindow.h \
    $$PWD/selectorder.h \
    $$PWD/csm_login.h

SOURCES += \
    $$PWD/XyAlgorithmDebug.cpp \
    $$PWD/XyMesDebug.cpp \
    $$PWD/XyScanDialog.cpp \
    $$PWD/loginwindow.cpp \
    $$PWD/main.cpp \
    $$PWD/mainwindow.cpp \
    $$PWD/selectorder.cpp \
    $$PWD/csm_login.cpp

