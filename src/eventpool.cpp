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
#include "eventpool.h"

#include <QDebug>

EventPool::EventPool()
{
}


void EventPool::addAppointment( Appointment* inApp )
{
    // empty Appointments should not exist
    if( inApp->m_eventVector.isEmpty() )
        return;

    // check, we don't read duplicates
    if( m_appointmentsRead.contains( inApp->m_uid ) )
        return;

    m_appointments.append( inApp );
    m_appointmentsRead.insert( inApp->m_uid  );

    for( const Event e : inApp->m_eventVector )
    {
        for( int year = e.m_startDt.date().year() ; year <= e.m_endDt.date().year(); year++)
        {
            if( m_eventMap.contains( year ) )
            {
                QVector<Event> &events = m_eventMap[year];
                events.append( e );
            }
            else
            {
                QVector<Event> events;
                events.append( e );
                m_eventMap.insert( year, events );
            }
        }
    }
}


void EventPool::updateAppointment( Appointment* inApp )
{
    removeAppointmentWithEventsById( inApp->m_uid );
    addAppointment( inApp );
}


void EventPool::removeAppointmentWithEventsById( const QString inUid )
{
    m_appointmentsRead.remove( inUid );

    for( int year : m_eventMap.keys() )
    {
        QVector<Event> events = m_eventMap.take( year );
        QVector<Event> newEvents;
        for( Event ev : events )
        {
            if( ev.m_uid != inUid )
                newEvents.append( ev );
        }
        m_eventMap.insert( year, newEvents );
        events.clear();
    }

    for( Appointment* app : m_appointments )
    {
        if( app->m_uid == inUid )
        {
            m_appointments.removeOne( app );
            delete app;
            break;
        }
    }
}


bool EventPool::haveAppointment( const QString inUid ) const
{
    return m_appointmentsRead.contains( inUid );
}


const Appointment* EventPool::appointment( const QString inUid ) const
{
    const Appointment* tmp;
    bool found = false;
    for( const Appointment* app : m_appointments )
    {
        if( app->m_uid == inUid )
        {
            found = true;
            tmp = app;
            break;
        }
    }
    if( found )
        return tmp;
    return nullptr;
}


void EventPool::addMarker( const int inMarkerYear )
{
    m_yearMarkers.insert( inMarkerYear );
}


bool EventPool::queryMarker( const int inMarkerYear ) const
{
    return m_yearMarkers.contains( inMarkerYear );
}


void EventPool::changeColor(const int inUserCalendarId, const QColor inNewColor)
{
    for( Appointment* app : m_appointments )
    {
        if( app->m_userCalendarId == inUserCalendarId )
        {
            for( Event &e : app->m_eventVector )
            {
                qDebug() << " somone got a new color";
                e.m_eventColor = inNewColor;
            }
        }
    }
}


QVector<Event> EventPool::eventsByYear( const int inYear ) const
{
    QVector<Event> events = m_eventMap.value( inYear );
    return events;
}


QVector<Event> EventPool::eventsByYearMonth( const int inYear, const int inMonth ) const
{
    QVector<Event> events = m_eventMap.value( inYear );
    if( events.isEmpty() )
        return events;
    QVector<Event> events_month;

    QDate firstOfMonth = QDate( inYear, inMonth, 1 );
    QDate lastOfMonth = QDate( inYear, inMonth, firstOfMonth.daysInMonth() );

    for( const Event e : events )
    {
        if( e.m_endDt.date() >= firstOfMonth and e.m_startDt.date() <= lastOfMonth )
            events_month.append( e );
    }
    return events_month;
}


QVector<Event> EventPool::eventsBy3Weeks( const QDate date )
{
    QDate firstOfRange = date;
    // @fixme: explicit week start
    firstOfRange = firstOfRange.addDays( 1 - firstOfRange.dayOfWeek() - 7 );
    QDate lastOfRange = firstOfRange.addDays( 20 );
    QVector<Event> events = m_eventMap.value( date.year() );

    // load other years, if we overlap a year-boundary
    if( firstOfRange.year() < date.year() )
        events.append( m_eventMap.value( firstOfRange.year() ) );
    if( lastOfRange.year() > date.year() )
        events.append( m_eventMap.value( lastOfRange.year() ) );

    QVector<Event> events3Weeks;
    for( const Event e : events )
    {
        if( e.m_endDt.date() >= firstOfRange and e.m_startDt.date() <= lastOfRange )
            events3Weeks.append( e );
    }
    return events3Weeks;
}


QVector<Event> EventPool::eventsByWeek( const QDate date )
{
    QDate firstOfRange = date;
    // @fixme: explicit week start
    firstOfRange = firstOfRange.addDays( 1 - firstOfRange.dayOfWeek() );
    QDate lastOfRange = firstOfRange.addDays( 6 );

    QVector<Event> events = m_eventMap.value( date.year() );
    // load other years, if we overlap a year-boundary
    if( firstOfRange.year() < date.year() )
        events.append( m_eventMap.value( firstOfRange.year() ) );
    if( lastOfRange.year() > date.year() )
        events.append( m_eventMap.value( lastOfRange.year() ) );

    QVector<Event> eventsWeeks;
    for( const Event e : events )
    {
        if( e.m_endDt.date() >= firstOfRange and e.m_startDt.date() <= lastOfRange )
            eventsWeeks.append( e );
    }
    return eventsWeeks;
}


QVector<Event> EventPool::eventsByDay( const QDate date )
{
    QVector<Event> events = m_eventMap.value( date.year() );
    QVector<Event> eventsDay;

    for( const Event e : events )
    {
        if( e.m_endDt.date() >= date and e.m_startDt.date() <= date )
            eventsDay.append( e );
    }
    return eventsDay;
}
