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
    appointmentmanager.cpp \
    ../icalreader/icalbody.cpp \
    ../icalreader/icalinterpreter.cpp \
    ../icalreader/parameter.cpp \
    ../icalreader/property.cpp \
    ../icalreader/standarddaylightcomponent.cpp \
    ../icalreader/valarmcomponent.cpp \
    ../icalreader/veventcomponent.cpp \
    ../icalreader/vfreebusycomponent.cpp \
    ../icalreader/vjournalcomponent.cpp \
    ../icalreader/vtimezonecomponent.cpp \
    ../icalreader/vtodocomponent.cpp \
    icalimportdialog.cpp \
    icalimportthread.cpp

HEADERS  += mainwindow.h \
    storage.h \
    usercalendar.h \
    usercalendarnew.h \
    datetime.h \
    appointmentmanager.h \
    ../icalreader/icalbody.h \
    ../icalreader/icalinterpreter.h \
    ../icalreader/parameter.h \
    ../icalreader/property.h \
    ../icalreader/standarddaylightcomponent.h \
    ../icalreader/valarmcomponent.h \
    ../icalreader/veventcomponent.h \
    ../icalreader/vfreebusycomponent.h \
    ../icalreader/vjournalcomponent.h \
    ../icalreader/vtimezonecomponent.h \
    ../icalreader/vtodocomponent.h \
    icalimportdialog.h \
    icalimportthread.h

FORMS    += ui/mainwindow.ui \
    ui/appointmentdialog.ui \
    ui/usercalendarnew.ui \
    ui/settingsdialog.ui \
    ui/calendarmanagerdialog.ui \
    ui/icalimportdialog.ui

RESOURCES += ../daylight.qrc \

DISTFILES += \
    todo.txt \
    database.txt \
    ../icalreader/todo.text
