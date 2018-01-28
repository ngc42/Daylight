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
#include "datetime.h"

#include <QStringList>

#include <QDebug>


DateTime::DateTime()
    :
      QDateTime(),
      m_isDate(false),
      m_isUTC(false)
{
}


DateTime::DateTime( const DateTime &inDt )
    :
      QDateTime( inDt )
{
    m_isDate = inDt.isDate();
    m_isUTC = inDt.isUtc();
}


DateTime::DateTime( const QDate &date, const QTime &time, const QTimeZone &timeZone )
    :
      QDateTime( date, time, timeZone )
{
    m_isDate = not time.isValid();
    m_isUTC = timeZone == QTimeZone::utc();
}


DateTime::DateTime( const QDate &date, const QTime &time, const QTimeZone &timeZone, const bool inIsDate )
    :
      DateTime( date, time, timeZone )
{
    m_isDate = inIsDate;
}


DateTime::DateTime(const QDate &date )
    :
      QDateTime( date )
{
    m_isDate = true;
    m_isUTC = true;
}


bool DateTime::readDateTime( QString inDtString )
{
    QString s = inDtString.toUpper();
    if( s.contains( 'T' ) )
    {
        QStringList list = s.split( 'T', QString::SkipEmptyParts );
        m_isDate = false;
        if( list.count() == 2 )
        {
            QString t = list.at(1);
            if( t.endsWith( 'Z' ) )
            {
                m_isUTC = true;
                t = t.left( t.size() - 1 );     // remove "Z"
            }
            bool ret = readDate( list.at(0) );
            ret = ret and readTime( t );
            if( m_isUTC )
                setTimeSpec( Qt::UTC );
            return ret;
        }
        return false;
    }
    // no "T", must be a date
    m_isDate = true;
    return readDate( s );
}


bool DateTime::readDateTime( QString inDtString, bool inIsDate )
{
    m_isDate = inIsDate;
    if( m_isDate )
        return readDate( inDtString );
    else
        return readDateTime( inDtString );
}


DateTime DateTime::addDays( int nDays ) const
{
    QDateTime t = QDateTime::addDays( nDays );
    if( isDate() )
        return DateTime( t.date() );
    return DateTime( t.date(), t.time(), t.timeZone() );
}


DateTime DateTime::addWeeks( int  nWeeks ) const
{
    return addDays( 7 * nWeeks );
}


DateTime DateTime::addMonths( int nMonths ) const
{
    QDateTime t = QDateTime::addMonths( nMonths );
    if( isDate() )
        return DateTime( t.date() );
    return DateTime( t.date(), t.time(), t.timeZone() );
}


DateTime DateTime::addYears( int nYears ) const
{
    QDateTime t = QDateTime::addYears( nYears );
    if( isDate() )
        return DateTime( t.date() );
    return DateTime( t.date(), t.time(), t.timeZone() );
}


QString DateTime::toString() const
{
    if( m_isDate )
        return QDateTime::toString( "yyyy-MM-dd" );
    return QDateTime::toString( "yyyy-MM-dd hh:mm:ss t");
}


QString DateTime::toDtString() const
{
    if( m_isDate )
        return QDateTime::toString( "yyyyMMdd" );
    return QDateTime::toString( "yyyyMMddThhmmss");
}


bool DateTime::readDate( QString inDateString )
{
    QDate d;
    d = d.fromString( inDateString, "yyyyMMdd" );
    if( d.isValid() )
    {
        setDate( d );
        return true;
    }
    return false;
}


bool DateTime::readTime( QString inTimeString )
{
    QTime t;
    t = t.fromString( inTimeString, "hhmmss" );
    if( t.isValid() )
    {
        setTime( t );
        return true;
    }
    return false;
}


DateTime DateTime::string2DateTime(const QString inDateTime, const QString inTimeZoneString )
{
    DateTime d;
    d.readDateTime( inDateTime );
    if( inTimeZoneString == 'Z' )
    {
        d.setTimeSpec( Qt::UTC );
        return d;
    }
    if( inTimeZoneString.count() == 0 )
        return d;
    QTimeZone tz( inTimeZoneString.toUtf8() );
    if( tz.isValid() )
        d.setTimeZone(tz);
    return d;
}


void DateTime::dateTime2Strings( const DateTime inDateTime, QString &dtString, QString &tzString )
{
    if( inDateTime.isDate() )
    {
        dtString = inDateTime.toDtString();
        tzString = "";
    }
    else
    {
        dtString = inDateTime.toDtString();
        if( inDateTime.isUtc() )
            tzString = "Z";
        else
        {
            QTimeZone tz = inDateTime.timeZone();
            if( tz.isValid() )
                tzString = QString( tz.id() );
            else
                tzString = "";
        }
    }
}
