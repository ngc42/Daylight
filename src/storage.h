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
#ifndef STORAGE_H
#define STORAGE_H

#include <QSqlDatabase>
#include <QVector>

#include "appointmentmanager.h"
#include "datetime.h"
#include "usercalendar.h"


/* This is the only storage class at the moment. It stores appointments and user calendars in a
 *  SQLITE database.
 */
struct Storage
{


public:
    explicit Storage();
    void createDatabase();

    // === appointments ===
    void storeAppointment( const Appointment &apmData );
    // @fixme: this algorithm does not care for userCalendarId:
    void updateAppointment( const Appointment &apmData );
    void loadAppointmentByYear(const int year , QVector<Appointment*> &outAppointments);
    void removeAppointment(const QString id);   // remove appointment from storage

    void setAppointmentsCalendar(const QString appointmentId, const int calendarId);

    void loadUserCalendarInfo( UserCalendarPool* &ucalPool );
    void insertUserCalendarInfo(const UserCalendarInfo* ucinfo);
    void userCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible);

    void removeUserCalendar(const int id);  // delete calendar and associated appointments

private:
    QSqlDatabase m_db;

    DateTime string2DateTime(const QString inDateTime, const QString inTimeZoneString );
    void dateTime2Strings( const DateTime inDateTime, QString &dtString, QString &tzString );
};

#endif // STORAGE_H
