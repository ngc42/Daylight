#include "eventpool.h"

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
                QVector<Event> events = m_eventMap[year];
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


bool EventPool::haveAppointment( const QString inUid ) const
{
    return m_appointmentsRead.contains( inUid );
}


void EventPool::addMarker( const int inMarkerYear )
{
    m_yearMarkers.insert( inMarkerYear );
}


bool EventPool::queryMarker( const int inMarkerYear ) const
{
    return m_yearMarkers.contains( inMarkerYear );
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
    for( const Event e : events )
    {
        if( e.m_startDt.date().month() <= inMonth and e.m_endDt.date().month() >= inMonth )
        {
            events_month.append( e );
            continue;
        }
        // @fixme: merge next both rules
        if( e.m_startDt.date().year() < inYear and e.m_endDt.date().year() > inYear )
        {
            events_month.append( e );
            continue;
        }
        if( e.m_startDt.date().year() < inYear and e.m_endDt.date().year() == inYear and
                e.m_endDt.date().month() >= inMonth )
        {
            events_month.append( e );
            continue;
        }
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
