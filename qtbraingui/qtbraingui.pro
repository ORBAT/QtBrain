#-------------------------------------------------
#
# Project created by QtCreator 2010-06-07T17:14:28
#
#-------------------------------------------------

QT       += core gui
TARGET = qtbrain

TEMPLATE = app

SOURCES += main.cpp\
        brainwindow.cpp

INCLUDEPATH += ../inc
LIBS += -L../bin \
        -lbfinterp

HEADERS  += brainwindow.h

FORMS    += brainwindow.ui
