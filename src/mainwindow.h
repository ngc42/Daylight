/*  Daylight - day planner, appointment book application
    Copyright (C) 2014  Eike Lange (eike(at)ngc42.de)

    Daylight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QToolButton>

#include "appointmentmanager.h"
#include "icalimportdialog.h"
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
    Ui::MainWindow* m_ui;
    Storage* m_storage;                         // database of appointments, storage on disk
    QActionGroup* m_groupCalendarAppearance;    // Year, Month, ...
    IcalImportDialog*   m_icalImportDialog;     // Dialog to read Ical files
    UserCalendarPool* m_userCalendarPool;       // Container for user calendars
    UserCalendarNew* m_userCalendarNewDialog;   // Dialog to add a user calendar

    QToolButton* m_toolbarDateLabel;            // label/button to show current selected date and start navigationDialog
    QToolButton* m_toolbarUserCalendarMenu;     // shows user calendars, switch them on/off
    void resizeCalendarView();                  // collector of resize events
    void showAppointments(const QDate &date);  // update appointments

protected:
    void resizeEvent(QResizeEvent* event);
    void moveEvent(QMoveEvent* event);

public slots:
    // file
    void slotOpenIcalFile();
    void slotLoadedAppointmentFromStorage(const Appointment);

    // User calendars
    void slotAddUserCalendar();
    void slotAddUserCalendarDlgFinished(int returncode);

};


#endif // MAINWINDOW_H
