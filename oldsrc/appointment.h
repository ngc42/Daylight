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
#ifndef APPOINTMENT_H
#define APPOINTMENT_H

#include <QDateTime>
#include <QColor>



/* current recurrences are: every year, month... or no recurrence */
enum RecurrenceType{R_NO_RECURRENCE, R_YEAR, R_MONTH, R_WEEK, R_DAY};



/* Recurence-data and Appointment-data are together the database data for an appointment. */
struct RecurrenceData
{
    int m_appointmentId;        // Every appointment has exact one Recurrence object, this is how we address it in db
    RecurrenceType m_type;      // type of recurrence
    bool m_forever;             // true: do not end the recurrence at some time, false: respect m_lastDt
    QDateTime m_lastDt;         // recurrence ends after this date, if m_forever is false
};



/* appointment basis data */
struct AppointmentData
{
    QString m_title;            // title to show
    int m_appointmentId;        // id, corresponds to RecurrendsData::m_appointmentId
    int m_userCalendarId;       // an id to connect to the UserCalendar, struct UserCalendarInfo::m_id
    int m_allDay;               // from morning to evening, all day long
    QDateTime m_startDt;        // start date and time, time is (should be) ignored, if m_allDay is true
    QDateTime m_endDt;          // end date and time
};



/* Appointments are created by a dialog in MainWindow (mainwindow.h) and destroyed by a context menu there.
 * The user can reconfigure (modify) an apointment.
 * Appointments consist of AppointmentData and Recurrence data. They add some functionality to check, if
 * a given date or date-range is affected by this appointment. This part is somewhat complicated, because
 * there are leap days, recurrences and yppointments over boudaries like years.
 * Appointments are managed by the AppointmentPool. */
class Appointment
{
public:

    Appointment(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & colorCopy);
    void modifyAppointment(const AppointmentData & newApmData, const RecurrenceData & newRecData, const QColor & newColor);

    // insertation sort AppointmentPool::add()
    bool sortsBefore(Appointment* & other) const;
    bool sortsSame(Appointment* & other) const;

    // true, if d is inside of range m_startDt...m_endDt
    bool containsDay(const QDate & d) const;
    // start and end are inclusive
    bool containsRange(const QDate & start, const QDate & end) const;


    bool sameDay() const { return m_sameDay; }
    qint64 numAffectedDays() const { return m_numAffectedDays; }
    QDateTime startDateTime() const { return m_appointmentData.m_startDt; }
    QDateTime endDateTime() const { return m_appointmentData.m_endDt; }

    AppointmentData m_appointmentData;
    RecurrenceData m_recurrenceData;
    QColor m_color;

private:
    bool m_sameDay;
    qint64 m_numAffectedDays;
};



/* Appointmel is the in app database for all appointments.
 * Appointments within this database can be modified, deleted and created, mostly
 * because of dialogs in MainWindow. On program start, AppointmentPool is populated
 * with available appointments from a database. The special function is addAppointmentFromStorage().
 * Whenever appointments need to be displayed, appointmentFor{Day, Week, ...} generates a list of
 * appointments based of Appointments::contains{Day, Range}.
 * fixme: the sorting does not make much sense, at the moment */
class AppointmentPool
{
public:
    AppointmentPool();
    int count() const { return m_pool.count(); }
    QList<AppointmentData> appointmentData() const;
    int add(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color);
    void modifyAppointment(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color);
    // called from mainwindow at startup
    void addAppointmentFromStorage(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color);
    
    void removeById(const int appointmentId);
    void removeByCalendarId(const int calendarId);
    void setColorForId(const int userCalendarId, const QColor & color);
    void setAppointmentCalendarForId(const int appointmentId, const int newCalendarId, const QColor & calendarColor);
    const QList<Appointment*> appointmentForRange(const QDate & start, const QDate & end) const;
    const QList<Appointment*> appointmentForDay(const QDate & day) const;
    const QList<Appointment*> appointmentForWeek(const QDate & day, int weekStart) const;
    const QList<Appointment*> appointmentFor3Weeks(const QDate & day, int weekStart) const;
    const QList<Appointment*> appointmentForMonth(const QDate & day, int weekStart) const;
    const QList<Appointment*> appointmentForYear(const QDate & day) const;

    Appointment* appointmentById(const int id) const;

private:
    void add(Appointment* & anApointment);    
    QList<Appointment*> m_pool;
    int m_nextAppointmentId;
};


#endif // APPOINTMENT_H
