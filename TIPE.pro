TEMPLATE = app
CONFIG += console c++20
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += \
     main.cpp\


HEADERS += \
    Affichage.h \
    ReseauMulticouche.h \

OTHER_FILES += \
    Affichage.tpp \
    ReseauMulticouche.tpp

win32: LIBS += -lGdi32

win32: LIBS += -luser32

win32: LIBS += -lshell32

win32:CONFIG(release, debug|release): LIBS += -L'C:/Program Files (x86)/libpng/lib/' -llibpng16
else:win32:CONFIG(debug, debug|release): LIBS += -L'C:/Program Files (x86)/libpng/lib/' -llibpng16d

INCLUDEPATH += 'C:/Program Files (x86)/libpng/include'
DEPENDPATH += 'C:/Program Files (x86)/libpng/include'
