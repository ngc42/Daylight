#-------------------------------------------------
#
# Project created by QtCreator 2014-07-18T13:01:55
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = daylightNg
TEMPLATE = app
CONFIG += c++11
# INCLUDEPATH += ../ui

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000


SOURCES += main.cpp\
        mainwindow.cpp \
    storage.cpp \
    usercalendar.cpp \
    usercalendarnew.cpp \
    datetime.cpp \
    appointmentmanager.cpp

HEADERS  += mainwindow.h \
    storage.h \
    usercalendar.h \
    usercalendarnew.h \
    datetime.h \
    appointmentmanager.h

FORMS    += ui/mainwindow.ui \
    ui/appointmentdialog.ui \
    ui/usercalendarnew.ui \
    ui/settingsdialog.ui \
    ui/calendarmanagerdialog.ui

RESOURCES += ../daylight.qrc \

DISTFILES += \
    todo.txt \
    database.txt
