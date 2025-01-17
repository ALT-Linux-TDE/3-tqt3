TEMPLATE = lib
TARGET	 = qmotifplusstyle

CONFIG  += qt warn_off release plugin
DESTDIR	 = ../../../styles

HEADERS		= ../../../../include/ntqmotifplusstyle.h
SOURCES		= main.cpp \
		  ../../../../src/styles/qmotifplusstyle.cpp

!contains(styles, motif) {
	HEADERS += ../../../../include/ntqmotifstyle.h
	SOURCES += ../../../../src/styles/qmotifstyle.cpp
}

unix:OBJECTS_DIR	= .obj
win32:OBJECTS_DIR	= obj

target.path += $$plugins.path/styles
INSTALLS += target
