  QT += xml

INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

include ($$PWD/qtpropertybrowser/qtpropertybrowser.pri)
include ($$PWD/qtpropertybrowser/qtpropertybrowserutils.pri)
include ($$PWD/qtpropertyfilepath/qtpropertyfilepath.pri)

SOURCES += $$PWD/CPropertyPage.cpp \
    $$PWD/CPropertyXml.cpp

HEADERS += $$PWD/CPropertyPage.h \
    $$PWD/CPropertyXml.h

RESOURCES += $$PWD/qtpropertybrowser/qtpropertybrowser.qrc
