#-------------------------------------------------
#
# Project created by QtCreator 2010-06-09T06:14:09
#
#-------------------------------------------------

QT       += core

QT       -= gui

TARGET = qtbrainconsole
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app


INCLUDEPATH += ../inc
LIBS += -L../bin \
        -lbfinterp

SOURCES += main.cpp \
    brainconsole.cpp

include(../qtbrain.pri)

HEADERS += \
    brainconsole.h
