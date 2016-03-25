#-------------------------------------------------
#
# Project created by QtCreator 2016-03-22T11:00:16
#
#-------------------------------------------------

QT       += core gui sql
CONFIG   += c++11

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = wrTime
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    wintimeedit.cpp

HEADERS  += mainwindow.h \
    wintimeedit.h

FORMS    += mainwindow.ui \
    wintimeedit.ui
