HEADERS = spheredata.h \
    fileinterface/colladainterface.h \
    fileinterface/tinyxml.h \
    fileinterface/tinystr.h \
    componenteditor/glbase.h \
    navigator/navigator.h \
    mainwindow.h \
    componenteditor/glwidget.h \
    componenteditor/tabeditor.h \
    propertybrowser/propertybrowser.h \
    propertybrowser/qteditorfactory.h \
    propertybrowser/qtpropertybrowser.h \
    propertybrowser/qtpropertybrowserutils_p.h \
    propertybrowser/qtpropertymanager.h \
    propertybrowser/qttreepropertybrowser.h
SOURCES = fileinterface/colladainterface.cc \
    fileinterface/tinyxml.cpp \
    fileinterface/tinyxmlerror.cpp \
    fileinterface/tinystr.cpp \
    fileinterface/tinyxmlparser.cpp \
    componenteditor/glwidget.cc \
    componenteditor/glbase.cc \
    navigator/navigator.cc \
    mainwindow.cc \
    componenteditor/tabeditor.cc \
    propertybrowser/propertybrowser.cpp \
    propertybrowser/qteditorfactory.cpp \
    propertybrowser/qtpropertybrowser.cpp \
    propertybrowser/qtpropertybrowserutils.cpp \
    propertybrowser/qtpropertymanager.cpp \
    propertybrowser/qttreepropertybrowser.cpp \
    main.cc
QT += opengl
LIBS += -lOpenCL \
    -lGLEW
CONFIG += debug
QMAKE_INCDIR += $(AMDAPPSDKROOT)/include
QMAKE_LIBDIR += $(AMDAPPSDKROOT)/lib/x86_64 \
    /usr/lib/fglrx

# install
target.path = .
sources.files = $$SOURCES \
    $$HEADERS \
    *.pro
sources.path = .
INSTALLS += target \
    sources
