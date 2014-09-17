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
INCLUDEPATH += source


SOURCES += source/main.cpp\
        source/mainwindow.cpp \
    source/calendarscene.cpp \
    source/calendarheader.cpp \
    source/dayitem.cpp \
    source/appointmentdialog.cpp \
    source/appointment.cpp \
    source/navigationdialog.cpp \
    source/usercalendar.cpp \
    source/usercalendarnew.cpp \
    source/storage.cpp \
    source/settingsdialog.cpp \
    source/calendarmanagerdialog.cpp

HEADERS  += source/mainwindow.h \
    source/calendarscene.h \
    source/calendarheader.h \
    source/dayitem.h \
    source/appointmentdialog.h \
    source/appointment.h \
    source/navigationdialog.h \
    source/usercalendar.h \
    source/usercalendarnew.h \
    source/storage.h \
    source/settingsdialog.h \
    source/calendarmanagerdialog.h

FORMS    += ui/mainwindow.ui \
    ui/appointmentdialog.ui \
    ui/usercalendarnew.ui \
    ui/settingsdialog.ui \
    ui/calendarmanagerdialog.ui

OTHER_FILES += \
    todo-DE.txt \
    gpl-3.0.txt

RESOURCES += \
    daylight.qrc
