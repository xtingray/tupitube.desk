TEMPLATE = app
CONFIG -= moc
TARGET = theora

LIBS += -ltheora -ltheoraenc -ltheoradec -logg

# Input
SOURCES += main.cpp
