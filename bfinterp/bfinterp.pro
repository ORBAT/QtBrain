#-------------------------------------------------
#
# Project created by QtCreator 2010-06-07T17:12:37
#
#-------------------------------------------------

QT       -= gui

TARGET = bfinterp
TEMPLATE = lib
VERSION = 0.01

DEFINES += BFINTERP_LIBRARY

SOURCES += bfinterpreter.cpp \
           bfinterpreter_p.cpp

PUBLIC_HEADERS += bfinterpreter.h \
                  bfinterp_global.h \
                  bihash.h

HEADERS += $$PUBLIC_HEADERS \
           bfinterpreter_p.h \
    customtransitions.h


include(../qtbrain.pri)
