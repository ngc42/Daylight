/*  This file is part of Kalendar-Reader.
Kalendar-Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kalendar-Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/
#ifndef DATETIME_H
#define DATETIME_H

#include <QDateTime>
#include <QString>
#include <QTimeZone>


/* This class is used to distinguish Dates from DateTimes, as required by the standard.
 * It uses a flag m_isDate and a method isDate() to show, if a value stored using this
 *  class is a date (then m_isDate is true) or a DateTime (then m_isDate is false).
 * The rest of this big thing is just there to make some functionality from QDateTime
 *  usable again.
 */
class DateTime : public QDateTime
{
public:
    // === constructors ===
    DateTime();
    DateTime( const DateTime &inDt );
    DateTime( const QDate &date, const QTime &time, const QTimeZone &timeZone );
    DateTime( const QDate &date, const QTime &time, const QTimeZone &timeZone, const bool inIsDate );
    DateTime(const QDate &date );

    // read the data
    bool readDateTime( QString inDtString );
    bool readDateTime( QString inDtString, bool inIsDate );

    // modify underlaying QDateTime
    DateTime addDays( int nDays ) const;
    DateTime addWeeks( int  nWeeks ) const;
    DateTime addMonths( int nMonths ) const;
    DateTime addYears( int nYears ) const;

    // === getter ===
    bool isDate() const { return m_isDate; }
    bool isUtc() const { return m_isUTC; }
    QString toString() const;
    QString toDtString() const;

    static DateTime string2DateTime(const QString inDateTime, const QString inTimeZoneString );
    static void dateTime2Strings( const DateTime inDateTime, QString &dtString, QString &tzString );

    // === operators ===
    DateTime &operator=( const DateTime &other )
    {
        m_isDate = other.isDate();
        m_isUTC = other.isUtc();
        setDate( other.date() );
        if( not m_isDate )
        {
            setTime( other.time() );
            setTimeZone( other.timeZone() );
        }
        return *this;
    }
    DateTime &operator=( const QDateTime &other )
    {
        setDate( other.date() );
        setTime( other.time() );
        setTimeZone( other.timeZone() );
        return *this;
    }

private:
    bool readDate( QString inDateString );
    bool readTime( QString inTimeString );

    // === Data ===
    bool        m_isDate;
    bool        m_isUTC;
};
#endif // DATETIME_H
