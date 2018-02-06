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

    for( Appointment* &app : m_appointments )
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


QVector<Event> EventPool::eventsByYearMonthDay( const int inYear, const int inMonth, const int inDay ) const
{
    QVector<Event> events = m_eventMap.value( inYear );
    if( events.isEmpty() )
        return events;
    QVector<Event> events_day;
    QDate targetDate(inYear, inMonth, inDay );
    for( const Event e : events )
    {
        if( e.m_startDt.date() <= targetDate and e.m_endDt.date()  >= targetDate )
            events_day.append( e );
    }
    return events_day;
}


QVector<Event> EventPool::eventsByYearMonthDayRange( const int inYearS, const int inMonthS, const int inDayS,
                                                     const int inYearE, const int inMonthE, const int inDayE) const
{

    int diff = inYearE - inYearS;
    if( diff != 0 or diff != 1 )
        return QVector<Event>();

    QVector<Event> events = m_eventMap.value( inYearS );
    if( inYearS != inYearE )
    {
        events.append( m_eventMap.value( inYearE ) );
    }
    if( events.isEmpty() )
        return events;
    QVector<Event> events_range;

    // start and end dates
    QDate targetDateS(inYearS, inMonthS, inDayS );
    QDate targetDateE(inYearE, inMonthE, inDayE );

    for( const Event e : events )
    {
        if( e.m_startDt.date() <= targetDateE and e.m_endDt.date()  >= targetDateS )
            events_range.append( e );
    }
    return events_range;
}

