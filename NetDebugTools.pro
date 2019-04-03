#-------------------------------------------------
#
# Project created by QtCreator 2019-04-03T08:46:20
#
#-------------------------------------------------

QT += core gui network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NetDebugTools
TEMPLATE = app


SOURCES += main.cpp
SOURCES += mainwindow.cpp

HEADERS  += mainwindow.h

FORMS    += mainwindow.ui

RESOURCES += res.qrc

RC_FILE = main.rc
