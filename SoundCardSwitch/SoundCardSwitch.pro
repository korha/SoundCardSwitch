TEMPLATE = app

QMAKE_LFLAGS += -static
QMAKE_CXXFLAGS += -Wpedantic

LIBS += -lgdi32 -lole32

SOURCES += main.cpp
