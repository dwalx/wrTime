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

TRANSLATIONS    = wrTime_ru.ts \
                  wrTime_en.ts

SOURCES += main.cpp\
        mainwindow.cpp \
    timevalidator.cpp

HEADERS  += mainwindow.h \
    timevalidator.h

FORMS    += mainwindow.ui

RESOURCES += \
    wrtime.qrc
