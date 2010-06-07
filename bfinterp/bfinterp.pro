#-------------------------------------------------
#
# Project created by QtCreator 2010-06-07T17:12:37
#
#-------------------------------------------------

QT       -= gui

TARGET = bfinterp
TEMPLATE = lib

DEFINES += BFINTERP_LIBRARY

SOURCES += bfinterpreter.cpp \
           bfinterpreter_p.cpp

PUBLIC_HEADERS += bfinterpreter.h

HEADERS += $$PUBLIC_HEADERS \
           bfinterp_global.h \
           bfinterpreter_p.h

include(../qtbrain.pri)
