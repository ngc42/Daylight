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
#ifndef CALENDARSCENE_H
#define CALENDARSCENE_H

#include <QGraphicsScene>
#include "calendarheader.h"
#include "dayitem.h"
#include "appointment.h"
#include "settingsdialog.h"



/* An enum to describe, what is actually shown in CalendarScene */
enum class CalendarShow {SHOW_UNKNOWN, SHOW_YEAR, SHOW_MONTH, SHOW_3WEEKS, SHOW_WEEK, SHOW_DAY};



/* CalendarScene is created in MainWindow. The purpose is to display the various ways of Calendars
 * with their days, which contain the appointments.
 * CalendarScene creates all the dates for every view and holds the current selected date.
 * The current show is started by SettingsData::m_last_view in main window and controlled by menu/toolbar
 * actions over slots like slotShowYear() and friends.
 * Each time, an appointment is created/modified or a usercalendar was modified, CalendarScene receives appointments
 * from AppointmentPool and has to dispatch all these in setAppointmentFor{Year, Month, ....}.
 * There are no own signals. Signals are taken from DayIn{Year, Month, ...}Item and forwarded to MainWindow. */
class CalendarScene : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit CalendarScene(const SettingsData & settings, QObject* parent = 0);
    void setDate(const QDate & date, bool update = false);
    QDate date() const { return m_currentBaseDate; }
    CalendarShow showView() const { return m_showView; }
    void updateSize(const QSize & newSize);
    void setAppointmentsForYear(const QList<Appointment*> & list);
    void setAppointmentsForMonth(const QList<Appointment*> & list);
    void setAppointmentsFor3Weeks(const QList<Appointment*> & list);
    void setAppointmentsForWeek(const QList<Appointment*> & list);
    void setAppointmentsForDay(const QList<Appointment*> & list);
    void setSettings(const SettingsData & settings);

private:
    // date
    QDate m_currentBaseDate;
    int m_weekStartDay;         // 1 = Mon, ..., 7 = Sun

    // header items
    void createHeader();
    QList<HeaderMonth*> m_headersMonth;
    QList<HeaderWeekday*> m_headersWeekday;
    QString m_longestMonthName;
    QString m_longestDayName;

    // day items
    void createDays();
    QList<DayInYearItem*> m_daysInYearItems;
    QList<DayInMonthItem*> m_daysInMonthItems;
    QList<DayInMonthItem*> m_daysIn3WeeksItems;
    QList<DayInWeekItem*> m_daysInWeekItems;
    DayInDayItem* m_dayInDayItem;

    QString m_longestDaylabelTextInYear;

    void hideAllItems();

    CalendarShow m_showView;

signals:
    void signalReconfigureAppointment(int appointmentId);
    void signalDeleteAppointment(int appointmentId);
    void signalDateClicked(const QDate & date);

public slots:
    void slotShowYear();
    void slotShowMonth();
    void slotShow3Weeks();
    void slotShowWeek();
    void slotShowDay();
};

#endif // CALENDARSCENE_H