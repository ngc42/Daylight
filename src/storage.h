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
#include "appointmentmanager.h"
#include "datetime.h"
#include "usercalendar.h"


/* This is the only storage class at the moment. It stores appointments and user calendars in a
 *  SQLITE database.
 * Reading from the database takes place ONLY at program start, while the data is written each
 *  time something is modified. Reading is done using the signals signalLoadedXXXFromStorage().
 * as this calendar program evolves, more and more sources of storage may be added.
 * There is a short circuit created in MainWindow:
 *  UserCalendarPoolsignalUserCalendarDataModified()  --> Storage::slotUserCalendarDataModified()
*/
class Storage : public QObject
{
    Q_OBJECT

public:
    explicit Storage(QObject* parent = 0);
    void createDatabase();
    void loadAppointmentData(const int year);
    void setAppointmentsCalendar(const QString appointmentId, const int calendarId);
    void removeAppointment(const QString id);   // remove appointment from storage
    void loadUserCalendarInfo();
    void insertUserCalendarInfo(const UserCalendarInfo* ucinfo);
    void removeUserCalendar(const int id);  // delete calendar and associated appointments

private:
    QSqlDatabase m_db;

    DateTime string2DateTime(const QString inDateTime, const QString inTimeZoneString );
    void dateTime2Strings( const DateTime inDateTime, QString &dtString, QString &tzString );


signals:
    void signalLoadedUserCalendarFromStorage(UserCalendarInfo* &info);
    void signalLoadedAppointmentFromStorage(const Appointment &apmData);

public slots:
    void slotAppointmentAdd(const Appointment &apmData);
    void slotUserCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible);
};

#endif // STORAGE_H
