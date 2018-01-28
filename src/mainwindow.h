/*  This file is part of Daylight.
    Daylight - Calendarmanager, Appointment-program
    Copyright (C) 2014-2018  E.Lange

    Daylight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License.

    Daylight is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>

#include "appointmentdialog.h"
#include "appointmentmanager.h"
#include "calendarscene.h"
#include "eventpool.h"
#include "icalimportdialog.h"
#include "navigationdialog.h"
#include "settingsdialog.h"
#include "storage.h"
#include "usercalendar.h"
#include "usercalendarnew.h"


namespace Ui {
    class MainWindow;
}


/* Representing the main window and same time controller of storage, appointment-pool,
 *  calendar-pool, calendar views, dialogs and much more. There is much to do for this class an
 *  dividing it into seperate managers could increase code readability.
 * There is a UI in mainwindow.ui available.
 * At startup, the storage (class Storage) fetches appointments and user calendars to user calendar pool and
 *  appointment pool. Then the setting manager looks up what to show. CalendarScene will then show up the
 *  current view with selected date and visible appointments.
 * There is an AppointmentDialog starting, whenever the scene wants to reconfigure or create an appointment and
 *  whenever the user selects something like "NEW APPOINTMENT" from the menu.
 * A navigation dialog with a small calendar and navigation buttons can shown up.
 * We have a "new calendar" dialog to set up a new calendar and a calendar manager so reconfigure a already,
 *  existing calendar, deleting a calendar and moving appointments from one calendar to the other. */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private:
    // Part UI:
    Ui::MainWindow*     m_ui;
    QActionGroup*       m_groupCalendarAppearance;  // Year, Month, ...
    QToolButton*        m_toolbarDateLabel;     // label/button to show current selected date and start navigationDialog
    QToolButton*        m_toolbarUserCalendarMenu;  // shows user calendars, switch them on/off
    CalendarScene*      m_scene;                // where we paint calendar in

    // Part Storage:
    Storage*            m_storage;              // database of appointments, storage on disk
    EventPool*          m_eventPool;            // database of events during runtime
    UserCalendarPool*   m_userCalendarPool;     // Container for user calendars

    // Part Dialogues:
    AppointmentDialog*  m_appointmentDialog;    // non modal dlg to set up appointments
    IcalImportDialog*   m_icalImportDialog;     // Dialog to read Ical files
    NavigationDialog*   m_navigationDialog;     // navigation dialog, shown in slotShowHideNavigationDlg()
    SettingsManager*    m_settingsManager;
    UserCalendarNew*    m_userCalendarNewDialog;    // Dialog to add a user calendar

    void showAppointments(const QDate &date);   // update appointments

protected:
    void resizeEvent(QResizeEvent*);
    void moveEvent(QMoveEvent* event);

public slots:
    // file
    void slotOpenIcalFile();
    void slotImportFromFileFinished();

    // set date
    void slotShowHideNavigationDlg();
    void slotSetDate(int year, int month);      // need to set a new Date
    void slotSetDate(const QDate & date);       // need to set a new Date
    void slotSetToday();
    void slotSetPreviousDate();
    void slotSetNextDate();

    // diferent ways of showing a calendar
    void slotShowYear();
    void slotShowMonth();
    void slotShow3Weeks();
    void slotShowWeek();
    void slotShowDay();

    // settings
    void slotSettingsDialog();

    // User calendars
    void slotAddUserCalendarDlg();
    void slotAddUserCalendarDlgFinished(int returncode);
    void slotCalendarManagerDialog();
    void slotModifyCalendar(const int calendarId, const QString & title, const QColor & color);
    void slotDeleteCalendar(const int calendarId);

    // appointments
    void slotAppointmentDlgStart(const QDate &date = QDate::currentDate());
    void slotReconfigureAppointment(QString appointmentId); // user clicks on an appointment, configure AppointmentDlg and start
    void slotAppointmentDlgFinished(int returncode);
};


#endif // MAINWINDOW_H
