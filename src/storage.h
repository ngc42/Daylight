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
#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include <QSqlDatabase>
#include <QVector>

#include "appointmentmanager.h"
#include "datetime.h"
#include "usercalendar.h"


/* This is the only storage class at the moment. It stores appointments and user calendars in a
 *  SQLITE database.
 */
class Storage : public QObject
{

    Q_OBJECT

public:
    explicit Storage();
    void createDatabase();

    // === appointments ===
    void storeAppointment( const Appointment* apmData );
    // @fixme: this algorithm does not care for userCalendarId:
    void updateAppointment( const Appointment* apmData );
    void loadAppointmentByYear( const int year, QVector<Appointment*> &outAppointments);
    void removeAppointment(const QString id);   // remove appointment from storage

    void setAppointmentsCalendar(const QString appointmentId, const int calendarId);

    void loadUserCalendarInfo( UserCalendarPool* &ucalPool );
    void insertUserCalendarInfo(const UserCalendarInfo* ucinfo);
    void userCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible);

    void removeUserCalendar(const int id);  // delete calendar and associated appointments

private:
    QSqlDatabase m_db;

signals:
    void sigStoreEvent(int first, int current, int count );
};

#endif // STORAGE_H
