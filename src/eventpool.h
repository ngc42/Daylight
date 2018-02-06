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
#ifndef EVENTPOOL_H
#define EVENTPOOL_H

#include "appointmentmanager.h"

#include <QColor>
#include <QMap>
#include <QSet>
#include <QVector>


class EventPool
{
public:
    EventPool();

    // Appointments
    void addAppointment( Appointment* inApp );
    void updateAppointment( Appointment* inApp );
    void removeAppointmentWithEventsById( const QString inUid );

    bool haveAppointment( const QString inUid ) const;
    const Appointment* appointment( const QString inUid ) const;

    /* year marker
     * Appointments can get requested fo special years. If we had such requests,
     * we mark this in a set, so we do not need duplicate database queries
    */
    void addMarker( const int inMarkerYear );
    bool queryMarker( const int inMarkerYear ) const;

    /* This traverses all stored appointments and all events within
     *  and changes the color.
     * You don't need to use it together with addAppointment(), as mainWindow
     *  cares for the color to show up. This is for changed colors during
     * dialogues and such. */
    void changeColor( const int inUserCalendarId, const QColor inNewColor );

    // Events
    QVector<Event> eventsByYear( const int inYear ) const;
    QVector<Event> eventsByYearMonth( const int inYear, const int inMonth ) const;
    QVector<Event> eventsBy3Weeks( const QDate date );
    QVector<Event> eventsByWeek( const QDate date );
    QVector<Event> eventsByDay( const QDate date );


private:
    QVector<Appointment*>       m_appointments;

    // set of uid, just to mark which ones we have, no duplicates.
    QSet<QString>               m_appointmentsRead;

    // set of years to make update easier, see above
    QSet<int>                   m_yearMarkers;

    QMap<int, QVector<Event>>   m_eventMap; // year and event list
};

#endif // EVENTPOOL_H
