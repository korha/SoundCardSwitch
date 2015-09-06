TEMPLATE = app

CONFIG -= app_bundle
CONFIG -= qt

QMAKE_LFLAGS += -static

LIBS += -lole32 -lgdi32

SOURCES += main.cpp
