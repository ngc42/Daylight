#-------------------------------------------------
#
# Project created by QtCreator 2014-07-18T13:01:55
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = daylight
TEMPLATE = app
CONFIG += c++11
# INCLUDEPATH += ../ui

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000


SOURCES += main.cpp\
        mainwindow.cpp \
    calendarscene.cpp \
    calendarheader.cpp \
    dayitem.cpp \
    appointmentdialog.cpp \
    appointment.cpp \
    navigationdialog.cpp \
    usercalendar.cpp \
    usercalendarnew.cpp \
    storage.cpp \
    settingsdialog.cpp \
    calendarmanagerdialog.cpp

HEADERS  += mainwindow.h \
    calendarscene.h \
    calendarheader.h \
    dayitem.h \
    appointmentdialog.h \
    appointment.h \
    navigationdialog.h \
    usercalendar.h \
    usercalendarnew.h \
    storage.h \
    settingsdialog.h \
    calendarmanagerdialog.h

FORMS    += ui/mainwindow.ui \
    ui/appointmentdialog.ui \
    ui/usercalendarnew.ui \
    ui/settingsdialog.ui \
    ui/calendarmanagerdialog.ui

RESOURCES += ../daylight.qrc \

DISTFILES += \
    todo.txt
