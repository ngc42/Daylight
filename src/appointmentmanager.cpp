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
#include "appointmentmanager.h"

#include <QDebug>
#include <QRandomGenerator>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

/* ***********************************************
 * ******* AppointmentBasics *********************
 * **********************************************/
AppointmentBasics::AppointmentBasics()
    :
      m_uid(""),
      m_sequence(1),
      m_summary(""),
      m_description("")
{
}


QString AppointmentBasics::contententToString() const
{
    QString a( "{AppointmentBasics: [%1][%2 - %3][%4][%5]}\n" );
    return a.arg( m_uid )
            .arg( m_dtStart.toString() )
            .arg( m_dtEnd.toString() )
            .arg( m_summary )
            .arg( m_description );
}


/* ***********************************************
 * ******* AppointmentAlarm **********************
 * **********************************************/
AppointmentAlarm::AppointmentAlarm()
    :
      m_alarmSecs(0),
      m_repeatNumber(0),
      m_pauseSecs(0)
{
}


QString AppointmentAlarm::contentToString() const
{
    QString a( "{AppointmentAlarm: [secs:%1][repeat:%2][pause:%3]}\n" );
    return a.arg( m_alarmSecs )
            .arg( m_repeatNumber )
            .arg( m_pauseSecs );
}


/* ***********************************************
 * ******* AppointmentRecurrence *****************
 * **********************************************/

AppointmentRecurrence::AppointmentRecurrence( QObject* parent )
    :
      QObject( parent ),
      m_haveCount(false),
      m_haveInterval(false),
      m_haveUntil(false),
      m_frequency(RFT_SIMPLE_YEARLY),
      m_count(0),
      m_interval(1),
      m_startWeekday(WD_MO)
{
}


QString AppointmentRecurrence::contentToString() const
{
    QString ed;
    if( m_exceptionDates.isEmpty() )
        ed = "[exdates:no]";
    else
    {
        for( DateTime d : m_exceptionDates )
            ed = ed.append( QString( "(ed:%1)").arg(d.toString() ) );
        ed = ed.prepend( "[exdates:");
        ed = ed.append( "]" );
    }

    QString fd;
    if( m_fixedDates.isEmpty() )
        fd = "[fixdates:no]";
    else
    {
        for( DateTime d : m_fixedDates )
            fd = fd.append( QString( "(fd:%1)").arg(d.toString() ) );
        fd = fd.prepend( "[fixdates:");
        fd = fd.append( "]" );
    }

    QString bds;
    if( m_byDaySet.size() == 0 )
        bds = "[bydaymap:no]";
    else
    {
        for( const std::pair<WeekDay,int> dayElem : m_byDaySet )
        {
             bds = bds.append( QString( "(%1:%2)" ).arg( static_cast<int>(dayElem.first) ).arg( dayElem.second ) );
        }

        bds = bds.prepend( "[bydaymap:" );
        bds = bds.append( "]" );
    }

    QString mdl;
    if( m_byMonthDaySet.isEmpty() )
        mdl = "[monthdayset:no]";
    else
    {
        for( const int i : m_byMonthDaySet )
            mdl = mdl.append( QString( "(mdl:%1)").arg( i ) );
        mdl = mdl.prepend( "[monthdayset:");
        mdl = mdl.append( "]" );
    }
    QString ydl;
    if( m_byYearDaySet.isEmpty() )
        ydl = "[yeardayset:no]";
    else
    {
        for( const int i : m_byYearDaySet )
            ydl = ydl.append( QString( "(ydl:%1)").arg( i ) );
        ydl = ydl.prepend( "[yeardaylist:");
        ydl = ydl.append( "]" );
    }
    QString wnl;
    if( m_byWeekNumberSet.isEmpty() )
        wnl = "[weekno:no]";
    else
    {
        for( const int i : m_byWeekNumberSet )
            wnl = wnl.append( QString( "(wno:%1)").arg(i) );
        wnl = wnl.prepend( "[weekno:");
        wnl = wnl.append( "]" );
    }
    QString bml;
    if( m_byMonthSet.isEmpty() )
        bml = "[months:no]";
    else
    {
        for( const int i : m_byMonthSet )
            bml = bml.append( QString( "(mo:%1)").arg(i) );
        bml = bml.prepend( "[months:");
        bml = bml.append( "]" );
    }
    QString hl;
    if( m_byHourSet.isEmpty() )
        hl = "[hours:no]";
    else
    {
        for( const int i : m_byHourSet )
            hl = hl.append( QString( "(h:%1)").arg(i) );
        hl = hl.prepend( "[hours:");
        hl = hl.append( "]" );
    }
    QString ml;
    if( m_byMinuteSet.isEmpty() )
        ml = "[minutes:no]";
    else
    {
        for( const int i : m_byMinuteSet )
            ml = ml.append( QString( "(m:%1)").arg(i) );
        ml = ml.prepend( "[minutes:");
        ml = ml.append( "]" );
    }
    QString sl;
    if( m_bySecondSet.isEmpty() )
        sl = "[seconds:no]";
    else
    {
        for( const int i : m_bySecondSet )
            sl = sl.append( QString( "(s:%1)").arg(i) );
        sl = sl.prepend( "[seconds:");
        sl = sl.append( "]" );
    }
    QString sp;
    if( m_bySetPosSet.isEmpty() )
        sp = "[setpos:no]";
    else
    {
        for( const int i : m_bySetPosSet )
            sp = sp.append( QString( "(sp:%1)").arg(i) );
        sp = sp.prepend( "[setpos:");
        sp = sp.append( "]" );
    }

    QString a( "{AppointmentRecurrence: [FREQ:%1][count:%2][interval:%3][until:%4][wd:%5]%6%7%8%9%10%11%12%13%14%15%16}\n" );
    return a.arg( static_cast<int>(m_frequency) )
            .arg( m_count )
            .arg( m_interval )
            .arg( m_until.toString() )
            .arg( static_cast<int>(m_startWeekday ) )
            .arg( ed )
            .arg( fd )
            .arg( bds )
            .arg( mdl )
            .arg( ydl )
            .arg( wnl )
            .arg( bml )
            .arg( hl ).arg( ml ).arg( sl )
            .arg( sp );
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDates( const DateTime inDtStart )
{
    DateTime lastDt;
    if( m_until.isValid() )
        lastDt = m_until;
    else
        lastDt.readDateTime( "21001231", true );

    qDebug() << "AppointmentRecurrence::recurrenceStartDates " << m_frequency << inDtStart.toDtString() ;

    if( m_frequency == RFT_SIMPLE_YEARLY )
        return recurrenceStartDatesSimpleYearly( inDtStart, lastDt );
    if( m_frequency == RFT_SIMPLE_MONTHLY )
        return recurrenceStartDatesSimpleMonthly( inDtStart, lastDt );
    if( m_frequency == RFT_SIMPLE_WEEKLY )
        return recurrenceStartDatesSimpleWeekly( inDtStart, lastDt );
    if( m_frequency == RFT_SIMPLE_DAILY )
        return recurrenceStartDatesSimpleDaily( inDtStart, lastDt );
    if( m_frequency == RFT_YEARLY )
        return recurrenceStartDatesYearly( inDtStart, lastDt );
    if( m_frequency == RFT_MONTHLY )
        return recurrenceStartDatesMonthly( inDtStart, lastDt );
    if( m_frequency == RFT_WEEKLY )
        return recurrenceStartDatesWeekly( inDtStart, lastDt );
    if( m_frequency == RFT_DAILY )
        return recurrenceStartDatesDaily( inDtStart, lastDt );

    return QVector<DateTime>();
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesSimpleYearly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    DateTime runner = inDtStart;
    while( runner <= inDtLast )
    {
        if( validateDateTime( runner ) )
            targetList.append( runner );
        runner = runner.addYears( m_interval );
        if( m_count > 0 )
        {
            if( targetList.count() >= m_count )
                break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesSimpleMonthly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    DateTime runner = inDtStart;
    while( runner <= inDtLast )
    {
        if( validateDateTime( runner ) )
            targetList.append( runner );
        runner = runner.addMonths( m_interval );
        if( m_count > 0 )
        {
            if( targetList.count() >= m_count )
                break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesSimpleWeekly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    DateTime runner = inDtStart;
    while( runner <= inDtLast )
    {
        if( validateDateTime( runner ) )
            targetList.append( runner );
        runner = runner.addDays( 7* m_interval );
        if( m_count > 0 )
        {
            if( targetList.count() >= m_count )
                break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesSimpleDaily( const DateTime inDtStart, const DateTime inDtLast )
{
    emit signalTick( inDtStart.date().year(), inDtStart.date().year(), inDtLast.date().year() );
    QVector<DateTime> targetList;
    DateTime runner = inDtStart;
    while( runner <= inDtLast )
    {
        emit signalTick( inDtStart.date().year(), runner.date().year(), inDtLast.date().year() );
        //qDebug() << "SEND: " << inDtStart.date().year() << " " << runner.date().year() << " " << inDtLast.date().year();
        if( validateDateTime( runner ) )
            targetList.append( runner );
        runner = runner.addDays( m_interval );
        if( m_count > 0 )
        {
            if( targetList.count() >= m_count )
                break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesYearly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    QVector<DateTime> yearTargetList;
    DateTime runner = inDtStart;

    bool have_byMonth =     not m_byMonthSet.isEmpty();
    bool have_byWeekNo =    not m_byWeekNumberSet.isEmpty();
    bool have_byYearDay =   not m_byYearDaySet.isEmpty();
    bool have_byMonthDay =  not m_byMonthDaySet.isEmpty();
    bool have_byDay =       m_byDaySet.size() > 0;
    bool have_byTime =      not (m_byHourSet.isEmpty() and
                                 m_byMinuteSet.isEmpty() and
                                 m_bySecondSet.isEmpty() );
    bool have_bySetPos =    not m_bySetPosSet.isEmpty();

    while( runner <= inDtLast )
    {
        if( have_byMonth )          // Expand BYMONTH
        {
            if( have_byMonthDay )   // BYMONTH + BYMONTHDAY
            {
                for( const int month : m_byMonthSet )
                {
                    QDate start( runner.date().year(), month, 1 );
                    QDate end( runner.date().year(), month, start.daysInMonth() );
                    for( const int monthDay : m_byMonthDaySet )
                    {
                        QDate d;
                        if( monthDay > 0 )  d = start.addDays( monthDay - 1);
                        else                d = end.addDays( monthDay + 1);
                        DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                        if( validateDateTime( dt ) )
                        {
                            if( have_byDay )    // BYMONTH + BYMONTHDAY + BYDAY
                            {
                                int weekDay = d.dayOfWeek();
                                if(  daysetContainsDay(m_byDaySet, static_cast<WeekDay>(weekDay) ) )
                                    yearTargetList.append( dt );
                            }
                            else                // BYMONTH + BYMONTHDAY without BYDAY
                            {
                                yearTargetList.append( dt );
                            }
                        }
                    }
                }
            } // End if have_byMonthDay
            else                                // BYMONTH without BYMONTHDAY
            {
                if( have_byDay )                // BYMONTH + BYDAY without BYMONTHDAY
                {
                    for( const int month : m_byMonthSet )
                    {
                        QDate start( runner.date().year(), month, 1 );
                        QDate end( runner.date().year(), month, start.daysInMonth() );
                        QDate d = start;
                        while( d <= end )
                        {
                            int weekDay = d.dayOfWeek();
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) and
                                validateDateTime( dt ) )
                                yearTargetList.append( dt );
                            d = d.addDays( 1 );
                        }
                    }
                }
                else                            // just BYMONTH, nothing else
                {
                    for( const int month : m_byMonthSet )
                    {
                        QDate d( runner.date().year(), month, runner.date().day() );
                        DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                        if( validateDateTime( dt ) )
                            yearTargetList.append( dt );
                    }
                }
            }
        } // end else
        else if( have_byMonthDay )          // EXPAND BYMONTHDAY
        {
            // for every month of this year...
            for( int month = 1; month < 12; month++ )
            {
                QDate start( runner.date().year(), month, 1 );
                QDate end( runner.date().year(), month, start.daysInMonth() );
                for( const int day : m_byMonthDaySet )
                {
                    QDate d;
                    if( day > 0 )
                        d = start.addDays( day - 1 );
                    else
                        d = end.addDays( day + 1 );
                    DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                    if( have_byDay )        // BYMONTHDAY + BYDAY
                    {
                        int weekDay = dt.date().dayOfWeek();
                        if( daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) )
                            continue;
                    }

                    if( validateDateTime( dt ) )
                        yearTargetList.append( dt );
                }
            }
        }
        else if( have_byWeekNo )            // Expand BYWEEKNO
        {
            QVector<DateTime> tempList;
            for( const int week : m_byWeekNumberSet )
                weekExpand( runner, week, tempList );
            if( have_byDay )                // BYWEEKNO + BYDAY
            {
                for( DateTime dt : tempList )
                {
                    int weekDay = dt.date().dayOfWeek();
                    if( not ( daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) and
                              validateDateTime( dt ) ) )
                        continue;
                    yearTargetList.append( dt );
                }
            }
            else
            {
                // implicit day, is maybe included in the standard
                // Read 3.3.10: "Information, not contained in the rule,necessary ... are derived from
                // the Start Time ("DTSTART") component attribute. "
                int weekDayStart = inDtStart.date().dayOfWeek();
                for( DateTime dt : tempList )
                {
                    int weekDay = dt.date().dayOfWeek();
                    if( not (( weekDay == weekDayStart ) and validateDateTime( dt )) )
                        continue;
                    yearTargetList.append( dt );
                }
            }
        }
        else if( have_byYearDay )       // Expand BYYEARDAY
        {
            QDate start(runner.date().year(), 1, 1);
            QDate end( runner.date().year(), 12, 31 );
            for( const int dayNum : m_byYearDaySet )
            {
                QDate d;
                if( dayNum > 0 )
                    d = start.addDays( dayNum - 1 );
                else
                    d = end.addDays( dayNum + 1 );
                DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                if( validateDateTime( dt ) )
                {
                    if( have_byDay )        // BYYEARDAY + BYDAY
                    {
                        int weekDay = dt.date().dayOfWeek();
                        if ( daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) )
                            yearTargetList.append( dt );

                    }
                    else                    // BYYEARDAY without BYDAY
                        yearTargetList.append( dt );
                }
            }
        }
        else if( have_byDay )
        {
            QDate start( runner.date().year(), 1, 1 );
            QDate end( runner.date().year(), 12, 31 );

            for( const std::pair<WeekDay, int> dayItem : m_byDaySet )
            {
                WeekDay weekDay = dayItem.first;
                const int relDay = dayItem.second;
                QDate d;

                if( relDay == 0 )
                {
                    // example: all mondays
                    d = start;
                    while( d <= end )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( validateDateTime( dt) )
                                yearTargetList.append( dt );
                            d = d.addDays( 7 );
                        }
                        else
                            d = d.addDays( 1 );
                    }
                }
                else if( relDay > 0 )
                {
                    int dCount = 0;
                    d = start;
                    while( d <= end )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            dCount++;
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( dCount == relDay and validateDateTime( dt ) )
                            {
                                yearTargetList.append( dt );
                                break;
                            }
                            d = d.addDays( 7 );
                        }
                        else
                            d = d.addDays( 1 );
                    }
                }
                else // relDay < 0
                {
                    int dCount = 0;
                    d = end;
                    while( d >= start )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            dCount++;
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( dCount == relDay and validateDateTime( dt ) )
                            {
                                yearTargetList.append( dt );
                                break;
                            }
                            d = d.addDays( -7 );
                        }
                        else
                            d = d.addDays( -1 );
                    }
                }
            }
        }
        // Expand BYHOUR, BYMINUTE and BYSECOND
        if( have_byTime )
        {
            QVector<QTime> timeList;
            QVector<DateTime> targetAndTimeMerged;
            timeExpand( runner, m_byHourSet, m_byMinuteSet, m_bySecondSet, timeList );
            for( DateTime dt : yearTargetList )
            {
                for( const QTime t : timeList )
                {
                    DateTime target( dt.date(), t, dt.timeZone(), runner.isDate() );
                    if( validateDateTime( target ) )
                        targetAndTimeMerged.append( target );
                }
            }
            if( not targetAndTimeMerged.isEmpty() )
                yearTargetList = targetAndTimeMerged;
        }

        // for BYSETPOS and COUNT, we need to have a sorted list
        sortDaytimeList( yearTargetList );

        // Limit BYSETPOS
        if( have_bySetPos )
        {
            QVector<DateTime> setPosList;
            for( const int pos : m_bySetPosSet )
            {
                if( pos > 0 and yearTargetList.count() >= pos )
                {
                    if( validateDateTime( yearTargetList.at( pos - 1 ) ) )
                        setPosList.append( yearTargetList.at( pos - 1 ) );
                }
                else if( pos < 0 and (-yearTargetList.count() <= pos ) )
                {
                    int index = yearTargetList.count() + pos;
                    if( validateDateTime( yearTargetList.at( index ) ) )
                        setPosList.append( yearTargetList.at( index ) );
                }
            }
            if( not setPosList.isEmpty() )
                yearTargetList = setPosList;
        }

        // here, we move everything after DTSTART to targetList
        while( not yearTargetList.isEmpty()  )
        {
            DateTime yt = yearTargetList.first();
            if( yt >= inDtStart )
                targetList.append( yt );
            yearTargetList.removeFirst();
        }

        runner = runner.addYears( m_interval );
        if( m_count > 0 and targetList.count() >= m_count )
        {
            while( targetList.count() > m_count )
                targetList.removeLast();
            break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesMonthly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    QVector<DateTime> monthTargetList;
    DateTime runner = inDtStart;
    bool have_byMonth =     not m_byMonthSet.isEmpty();
    bool have_byMonthDay =  not m_byMonthDaySet.isEmpty();
    bool have_byDay =       m_byDaySet.size() > 0;
    bool have_byTime =      not (m_byHourSet.isEmpty() and
                                 m_byMinuteSet.isEmpty() and
                                 m_bySecondSet.isEmpty() );
    bool have_bySetPos =    not m_bySetPosSet.isEmpty();

    while( runner <= inDtLast )
    {
        if( have_byMonth )          // limit BYMONTH
        {
            bool validMonth = m_byMonthSet.contains( runner.date().month() );
            if( not validMonth )
            {
                runner = runner.addMonths( m_interval );
                if( inDtLast.date().year() < runner.date().year() )
                    break;
                continue;
            }
        }
        if( have_byMonthDay )           // expand BYMONTHDAY
        {
            QDate start( runner.date().year(), runner.date().month(), 1 );
            QDate end( start.year(), start.month(), start.daysInMonth() );
            for( const int day : m_byMonthDaySet )
            {
                QDate d;
                if( day > 0 )
                    d = start.addDays( day - 1 );
                else
                    d = end.addDays( day + 1 );
                DateTime dt = DateTime( d, runner.time(), runner.timeZone(), runner.isDate() );
                if( have_byDay )        // limit BYDAY, because BYMONTHDAY is present
                {
                    int weekDay = dt.date().dayOfWeek();
                    if( not daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) )
                        continue;
                }
                if( validateDateTime( dt ) )
                    monthTargetList.append( dt );
            }
        }
        else if( have_byDay )           // expand BYDAY
        {

            QDate start( runner.date().year(), runner.date().month(), 1 );
            QDate end( start.year(), start.month(), start.daysInMonth() );

            for( const std::pair<WeekDay, int> dayItem : m_byDaySet )
            {
                WeekDay weekDay = dayItem.first;
                const int relDay = dayItem.second;
                QDate d;

                if( relDay == 0)
                {
                    d = start;
                    while( d <= end )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( validateDateTime( dt ) )
                                monthTargetList.append( dt );
                            d = d.addDays( 7 );
                        }
                        else
                            d = d.addDays( 1 );
                    }
                }
                else if( relDay > 0 )
                {
                    d = start;
                    int dCount = 0;
                    while( d <= end )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            dCount++;
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( dCount == relDay and validateDateTime( dt ) )
                            {
                                monthTargetList.append( dt );
                                break;
                            }
                            d = d.addDays( 7 );
                        }
                        else
                            d = d.addDays( 1 );
                    }
                }
                else    // relDay < 0
                {
                    d = end;
                    int dCount = 0;
                    while( d >= start )
                    {
                        if( d.dayOfWeek() == static_cast<int>(weekDay) )
                        {
                            dCount--;
                            DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                            if( dCount == relDay and validateDateTime( dt ) )
                            {
                                monthTargetList.append( dt );
                                break;
                            }
                            d = d.addDays( -7 );
                        }
                        else
                            d = d.addDays( -1 );
                    }
                }
            }
        }

        // Expand BYHOUR, BYMINUTE and BYSECOND
        if( have_byTime )
        {
            QVector<QTime> timeList;
            QVector<DateTime> targetAndTimeMerged;
            timeExpand( runner, m_byHourSet, m_byMinuteSet, m_bySecondSet, timeList );
            for( DateTime dt : monthTargetList )
            {
                for( const QTime t : timeList )
                {
                    DateTime target( dt.date(), t, dt.timeZone(), runner.isDate() );
                    if( validateDateTime( target ) )
                        targetAndTimeMerged.append( target );
                }
            }
            if( not targetAndTimeMerged.isEmpty() )
                monthTargetList = targetAndTimeMerged;
        }

        // for BYSETPOS and COUNT, we need to have a sorted list
        sortDaytimeList( monthTargetList );

        // Limit BYSETPOS
        if( have_bySetPos )
        {
            QVector<DateTime> setPosList;
            for( const int pos : m_bySetPosSet )
            {
                if( pos > 0 and monthTargetList.count() >= pos )
                {
                    if( validateDateTime( monthTargetList.at( pos - 1 ) ) )
                        setPosList.append( monthTargetList.at( pos - 1 ) );
                }
                else if( pos < 0 and (-monthTargetList.count() <= pos ) )
                {
                    int index = monthTargetList.count() + pos;
                    if( validateDateTime( monthTargetList.at( index ) ) )
                        setPosList.append( monthTargetList.at( index ) );
                }
            }
            if( not setPosList.isEmpty() )
                monthTargetList = setPosList;
        }

        // here, we move everything after DTSTART to targetList
        while( not monthTargetList.isEmpty()  )
        {
            DateTime yt = monthTargetList.first();
            if( yt >= inDtStart )
                targetList.append( yt );
            monthTargetList.removeFirst();
        }

        runner = runner.addMonths( m_interval );
        if( m_count > 0 and targetList.count() >= m_count )
        {
            while( targetList.count() > m_count )
                targetList.removeLast();
            break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesWeekly( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    QVector<DateTime> weekTargetList;
    DateTime runner = inDtStart;

    bool have_byMonth =     not m_byMonthSet.isEmpty();
    bool have_byDay =       m_byDaySet.size() > 0;
    bool have_byTime =      not (m_byHourSet.isEmpty() and
                                 m_byMinuteSet.isEmpty() and
                                 m_bySecondSet.isEmpty() );
    bool have_bySetPos =    not m_bySetPosSet.isEmpty();

    while( runner <= inDtLast )
    {
        if( have_byMonth )          // limit BYMONTH
        {
            bool validMonth = m_byMonthDaySet.contains( runner.date().month() );
            if( not validMonth )
            {
                runner = runner.addWeeks( m_interval );
                if( inDtLast.date().year() < runner.date().year() )
                    break;
                continue;
            }
        }
        QDate weekStartDate = runner.date();
        // first day of this week
        while( weekStartDate.dayOfWeek() != static_cast<int>(m_startWeekday) )
            weekStartDate = weekStartDate.addDays( -1 );
        if( have_byDay )                //  BYDAY
        {
            QDate d = weekStartDate;
            // for every day of this week...
            for( int i = 0; i < 7; i++ )
            {
                int weekDay = d.dayOfWeek();
                if( daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) )
                {
                    DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                    if( validateDateTime( dt ) )
                        weekTargetList.append( dt );
                }
                d = d.addDays( 1 );
            }
        }
        else
        {
            // implicit day, is maybe included in the standard
            // Read 3.3.10: "Information, not contained in the rule,necessary ... are derived from
            // the Start Time ("DTSTART") component attribute. "
            int weekStartDay = inDtStart.date().dayOfWeek();
            QDate d = weekStartDate;
            for( int i = 0; i < 7; i++ )
            {
                int weekDay = d.dayOfWeek();
                if( weekDay == weekStartDay )
                {
                    DateTime dt( d, runner.time(), runner.timeZone(), runner.isDate() );
                    if( validateDateTime( dt ) )
                        weekTargetList.append( dt );
                    break;  // there is only one day in this week to match
                }
                d = d.addDays( 1 );
            }
        }

        // Expand BYHOUR, BYMINUTE and BYSECOND
        if( have_byTime )
        {
            QVector<QTime> timeList;
            QVector<DateTime> targetAndTimeMerged;
            timeExpand( runner, m_byHourSet, m_byMinuteSet, m_bySecondSet, timeList );
            for( DateTime dt : weekTargetList )
            {
                for( const QTime t : timeList )
                {
                    DateTime target( dt.date(), t, dt.timeZone(), runner.isDate() );
                    if( validateDateTime( target ) )
                        targetAndTimeMerged.append( target );
                }
            }
            if( not targetAndTimeMerged.isEmpty() )
                weekTargetList = targetAndTimeMerged;
        }

        // for BYSETPOS and COUNT, we need to have a sorted list
        sortDaytimeList( weekTargetList );

        // Limit BYSETPOS
        if( have_bySetPos )
        {
            QVector<DateTime> setPosList;
            for( const int pos : m_bySetPosSet )
            {
                if( pos > 0 and weekTargetList.count() >= pos )
                {
                    if( validateDateTime( weekTargetList.at( pos - 1 ) ) )
                        setPosList.append( weekTargetList.at( pos - 1 ) );
                }
                else if( pos < 0 and (-weekTargetList.count() <= pos ) )
                {
                    int index = weekTargetList.count() + pos;
                    if( validateDateTime( weekTargetList.at( index ) ) )
                        setPosList.append( weekTargetList.at( index ) );
                }
            }
            if( not setPosList.isEmpty() )
                weekTargetList = setPosList;
        }

        // here, we move everything after DTSTART to targetList
        while( not weekTargetList.isEmpty()  )
        {
            DateTime yt = weekTargetList.first();
            if( yt >= inDtStart )
                targetList.append( yt );
            weekTargetList.removeFirst();
        }

        runner = runner.addWeeks( m_interval );
        if( m_count > 0 and targetList.count() >= m_count )
        {
            while( targetList.count() > m_count )
                targetList.removeLast();
            break;
        }
    }
    return targetList;
}


QVector<DateTime> AppointmentRecurrence::recurrenceStartDatesDaily( const DateTime inDtStart, const DateTime inDtLast )
{
    QVector<DateTime> targetList;
    QVector<DateTime> dayTargetList;
    DateTime runner = inDtStart;
    bool have_byMonth =     not m_byMonthSet.isEmpty();
    bool have_byMonthDay =  not m_byMonthDaySet.isEmpty();
    bool have_byDay =       m_byDaySet.size() > 0;
    bool have_byTime =      not (m_byHourSet.isEmpty() and
                                 m_byMinuteSet.isEmpty() and
                                 m_bySecondSet.isEmpty() );
    bool have_bySetPos =    not m_bySetPosSet.isEmpty();

    emit signalTick( inDtStart.date().year(), inDtStart.date().year(), inDtLast.date().year() );

    while( runner <= inDtLast )
    {
        emit signalTick( inDtStart.date().year(), runner.date().year(), inDtLast.date().year() );

        if( have_byMonth )          // limit BYMONTH
        {
            bool validMonth = m_byMonthSet.contains( runner.date().month() );
            if( not validMonth )
            {
                runner = runner.addDays( m_interval );
                if( inDtLast.date() < runner.date() )
                    break;
                continue;
            }
        }

        if( have_byMonthDay )       // limit BYMONTHDAY
        {
            QDate startDate( runner.date().year(), runner.date().month(), 1 );
            QDate endDate( startDate.year(), startDate.month(), startDate.daysInMonth() );
            QDate d;
            bool found = false;
            for( const int relDay : m_byMonthDaySet )
            {
                if( relDay > 0 )
                {
                    d = startDate.addDays( relDay - 1 );
                    if( d == runner.date() )
                    {
                        found = true;
                        break;
                    }
                }
                else if( relDay < 0 )
                {
                    d = endDate.addDays( relDay + 1 );
                    if( d == runner.date() )
                    {
                        found = true;
                        break;
                    }
                }
            }
            if( not found )
            {
                runner = runner.addDays( m_interval );
                if( inDtLast.date() < runner.date() )
                    break;
                continue;
            }
        }

        if( have_byDay )        // limit BYDAY
        {
            int weekDay = runner.date().dayOfWeek();
            if( not daysetContainsDay( m_byDaySet, static_cast<WeekDay>(weekDay) ) )
            {
                runner = runner.addDays( m_interval );
                if( inDtLast.date() < runner.date() )
                    break;
                continue;
            }
        }

        // ... and all other cases: append the date
        if( validateDateTime( runner ) )
            dayTargetList.append( runner);

        // Expand BYHOUR, BYMINUTE and BYSECOND
        if( have_byTime )
        {
            QVector<QTime> timeList;
            QVector<DateTime> targetAndTimeMerged;
            timeExpand( runner, m_byHourSet, m_byMinuteSet, m_bySecondSet, timeList );
            for( DateTime dt : dayTargetList )
            {
                for( const QTime t : timeList )
                {
                    DateTime target( dt.date(), t, dt.timeZone(), runner.isDate() );
                    if( validateDateTime( target ) )
                        targetAndTimeMerged.append( target );
                }
            }
            if( not targetAndTimeMerged.isEmpty() )
                dayTargetList = targetAndTimeMerged;
        }

        // for BYSETPOS and COUNT, we need to have a sorted list
        sortDaytimeList( dayTargetList );

        // Limit BYSETPOS
        if( have_bySetPos )
        {
            QVector<DateTime> setPosList;
            for( const int pos : m_bySetPosSet )
            {
                if( pos > 0 and dayTargetList.count() >= pos )
                {
                    if( validateDateTime( dayTargetList.at( pos - 1 ) ) )
                        setPosList.append( dayTargetList.at( pos - 1 ) );
                }
                else if( pos < 0 and (-dayTargetList.count() <= pos ) )
                {
                    int index = dayTargetList.count() + pos;
                    if( validateDateTime( dayTargetList.at( index ) ) )
                        setPosList.append( dayTargetList.at( index ) );
                }
            }
            if( not setPosList.isEmpty() )
                dayTargetList = setPosList;
        }

        // here, we move everything after DTSTART to targetList
        while( not dayTargetList.isEmpty()  )
        {
            DateTime yt = dayTargetList.first();
            if( yt >= inDtStart )
                targetList.append( yt );
            dayTargetList.removeFirst();
        }

        runner = runner.addDays( m_interval );
        if( m_count > 0 and targetList.count() >= m_count )
        {
            while( targetList.count() > m_count )
                targetList.removeLast();
            break;
        }
    }
    return targetList;
}


void AppointmentRecurrence::weekExpand( const DateTime inRefDateTime, const int inWeekNo, QVector<DateTime> &outDayList )
{
    if( inWeekNo > 0 )
    {
        QDate wkStDate = firstDayOfWeek( inRefDateTime.date().year(), inWeekNo );
        for( int i = 0; i < 7; i++ )
        {
            DateTime aDay( wkStDate.addDays( i ), inRefDateTime.time(), inRefDateTime.timeZone(), inRefDateTime.isDate() );
            if( validateDateTime( aDay ) )
                outDayList.append( aDay );
        }
    }
    else
    {
        QDate wkStDate = firstDayOfWeek( inRefDateTime.date().year() + 1, 1);
        wkStDate = wkStDate.addDays( inWeekNo * 7 );
        for( int i = 0; i < 7; i++ )
        {
            DateTime aDay( wkStDate.addDays( i ), inRefDateTime.time(), inRefDateTime.timeZone(), inRefDateTime.isDate() );
            if( validateDateTime( aDay ) )
                outDayList.append( aDay );
        }
    }
}


void AppointmentRecurrence::timeExpand(const DateTime inRefDateTime, const QSet<int> inHourSet, const QSet<int> inMinSet,
                           const QSet<int> inSecSet, QVector<QTime> &outTimeList)
{
    QTime refTime = inRefDateTime.time();

    // append hours, if available. Else append a default hour
    QVector<QTime> t_hour;
    if( inHourSet.isEmpty() )
        t_hour.append( QTime( refTime.hour(), refTime.minute(), refTime.second() ) );
    else
        for( const int h : inHourSet )
        {
            QTime elem( h, refTime.minute(), refTime.second() );
            t_hour.append( elem );
        }

    // cross with minutes, if available. Else set minutes by default minute
    QVector<QTime> t_min;
    if( inMinSet.isEmpty())
    {
        for( const QTime t : t_hour )
        {
            QTime elem( t.hour(), refTime.minute(), refTime.second() );
            t_min.append( elem );
        }
    }
    else
    {
        for( const QTime t : t_hour )
        {
            for( const int m : inMinSet )
            {
                QTime elem( t.hour(), m, refTime.second() );
                t_min.append( elem );
            }
        }
    }

    // cross with seconds, else return t_min, as there are default values for
    //   missing seconds
    if( inSecSet.isEmpty() )
    {
        outTimeList = t_min;
        return;
    }
    for( const QTime t : t_min )
    {
        for( const int s : inSecSet )
        {
            QTime elem( t.hour(), t.minute(), s );
            outTimeList.append( elem );
        }
    }
}


QDate AppointmentRecurrence::firstDayOfWeek( const int inYear, const int inWeekNumber ) const
{
    QDate d( inYear, 1, 1 );
    const int dow = d.dayOfWeek();
    const int weekdayStart = static_cast<int>(m_startWeekday);
    // calculate first day of first week
    QDate start = d.addDays( -dow + weekdayStart );
    int z = start.daysTo( QDate(inYear, 1, 1) );
    if( z == 0 or z == 1 or z == 2 or z == 3 )
        return start.addDays( 7 * ( inWeekNumber - 1) );
    return start.addDays( 7 * inWeekNumber);
}


bool AppointmentRecurrence::validateDateTime( const DateTime inRefTime ) const
{
    // reject invalid dates
    if( not inRefTime.isValid() )
        return false;
    if( m_exceptionDates.count() > 0 )
    {
        for( const DateTime dt : m_exceptionDates )
        {
            if( inRefTime == dt or
                ( dt.isDate() and inRefTime.date() == dt.date() ) )
                return false;
        }
    }
    // because of a stupid BYSET-example in the standard, we cannot reject dates too early
    // if inRefDateTime < m_startDaytime then return false
    return true;
}


void AppointmentRecurrence::sortDaytimeList( QVector<DateTime> &inoutSortList )
{
    int count = inoutSortList.count();
    if( count < 2 )
        return;
    if( count == 2 )
    {
        if( inoutSortList.at(0) > inoutSortList.at(1) )
        {
            DateTime tmp = inoutSortList.at(0);
            inoutSortList[0] = inoutSortList.at(1);
            inoutSortList[1] = tmp;
        }
        return;
    }
    int index = 0;
    int bindex = count / 2;
    DateTime tmp;

    // bigger distance
    while( bindex < count )
    {
        if( inoutSortList.at( index ) > inoutSortList.at( bindex ) )
        {
            tmp = inoutSortList.at( index );
            inoutSortList[ index ] = inoutSortList[ bindex ];
            inoutSortList[ bindex ] = tmp;
        }
        index++;
        bindex++;
    }

    bool swapped = true;

    // bubblesort over short distance
    while( swapped )
    {
        swapped = false;
        index = 0;
        bindex = 1;
        while( bindex < count )
        {
            if( inoutSortList.at( index ) > inoutSortList.at( bindex ) )
            {
                swapped = true;
                tmp = inoutSortList.at( index );
                inoutSortList[ index ] = inoutSortList[ bindex ];
                inoutSortList[ bindex ] = tmp;
            }
            index++;
            bindex++;
        }
    }
}


/* ***********************************************
 * ******* Appointment ***************************
 * **********************************************/

Appointment::Appointment( QObject* parent )
    :
      QObject(parent),
      m_haveRecurrence( false ),
      m_haveAlarm( false )
{

}


void Appointment::generateUid()
{
    QDateTime dt = QDateTime::currentDateTime().toUTC();
    m_uid = dt.toString( "yyyyMMddTHHmmssZ-zzz" ) +
            QString( "-%1-DAILIGHT" )
            .arg( QRandomGenerator::global()->generate64() , 0, 16 )
            .toUpper();
}


void Appointment::makeDateList( const QString inElementsString, const QString inTimeZone, QList<DateTime> &outList )
{
    outList.clear();
    for( const QString s : inElementsString.split( ',', QString::SkipEmptyParts ) )
    {
        DateTime dt;
        dt.readDateTime( s );
        if( inTimeZone.count() > 0 )
        {
            QTimeZone tz( inTimeZone.toUtf8() );
            if( tz.isValid() )
                dt.setTimeZone(tz);
        }
        outList.append( dt );
    }
}


void Appointment::makeDayset( const QString inElementsString, std::set<std::pair<AppointmentRecurrence::WeekDay, int>> &outSet )
{
    outSet.clear();
    bool ok = false;
    QRegularExpression re( "([+-]?\\d*)(\\D*)" );
    AppointmentRecurrence::WeekDay weekDay = AppointmentRecurrence::WD_MO;
    for( const QString elem : inElementsString.split( ',', QString::SkipEmptyParts ) )
    {
        QRegularExpressionMatch match = re.match( elem );
        QString vString = match.captured( 1 );
        QString dString = match.captured( 2 );

        int value = 0;
        if( vString.size() > 0 )
        {
            // value is optional
            value = vString.toInt( &ok );
        }
        if( dString == "MO" )       weekDay = AppointmentRecurrence::WD_MO;
        else if( dString == "TU" )  weekDay = AppointmentRecurrence::WD_TU;
        else if( dString == "WE" )  weekDay = AppointmentRecurrence::WD_WE;
        else if( dString == "TH" )  weekDay = AppointmentRecurrence::WD_TH;
        else if( dString == "FR" )  weekDay = AppointmentRecurrence::WD_FR;
        else if( dString == "SA" )  weekDay = AppointmentRecurrence::WD_SA;
        else                        weekDay = AppointmentRecurrence::WD_SU;
        outSet.insert( std::make_pair( weekDay, value ) );
    }
}


void Appointment::makeIntSet( const QString inElementsString, QSet<int> &outSet )
{
    outSet.clear();
    bool ok;
    for( const QString s : inElementsString.split( ',', QString::SkipEmptyParts ) )
    {
        outSet.insert( s.toInt(&ok) );
    }
}


void Appointment::makeStringsFromDateList( const QList<DateTime> &inList, QString &outDtString, QString &outTzString )
{
    int count = inList.count();
    int num = 0;
    bool have_tz = false;
    outDtString = "";
    outTzString = "";
    for( const DateTime dt : inList )
    {
        QString s;
        num++;
        if(num != count )
            s = QString( "%1,").arg( dt.toDtString() );
        else
            s = QString( "%1").arg( dt.toDtString() );
        outDtString = outDtString.append(s);
        if( not ( dt.isDate() or have_tz ) )
        {
            if( dt.isUtc() )
                outTzString = "Z";
            else
            {
                QTimeZone tz = dt.timeZone();
                if( tz.isValid() )
                    outTzString = QString( tz.id() );
            }
            have_tz = true;
        }
    }
}


void Appointment::makeStringFromDayset( const std::set<std::pair<AppointmentRecurrence::WeekDay, int>> inSet, QString &outString )
{
    outString = "";
    int count = inSet.size();
    int num = 0;
    for( const std::pair<AppointmentRecurrence::WeekDay, int> dayItem : inSet )
    {
        QString weekDayAbbrev;
        AppointmentRecurrence::WeekDay weekDay = dayItem.first;
        int relDay = dayItem.second;
        switch( weekDay )
        {
            case AppointmentRecurrence::WD_MO:  weekDayAbbrev = "MO"; break;
            case AppointmentRecurrence::WD_TU:  weekDayAbbrev = "TU"; break;
            case AppointmentRecurrence::WD_WE:  weekDayAbbrev = "WE"; break;
            case AppointmentRecurrence::WD_TH:  weekDayAbbrev = "TH"; break;
            case AppointmentRecurrence::WD_FR:  weekDayAbbrev = "FR"; break;
            case AppointmentRecurrence::WD_SA:  weekDayAbbrev = "SA"; break;
            case AppointmentRecurrence::WD_SU:  weekDayAbbrev = "SU"; break;
        }

        num++;
        QString s;
        if( num != count )
            // strings are (-1MO), (0TU) and so on.
            s = QString( "%1%2,").arg( relDay ).arg( weekDayAbbrev );
        else
            s = QString( "%1%2").arg( relDay ).arg( weekDayAbbrev );
        outString += s;
    }
}


void Appointment::makeStringFromIntSet( const QSet<int> inIntSet, QString &outString )
{
    outString = "";
    int count = inIntSet.count();
    int num = 0;
    for( const int i : inIntSet )
    {
        QString s;
        num++;
        if( num != count )
            s = QString("%1,").arg( i );
        else
            s = QString("%1").arg( i );
        outString = outString.append(s);
    }
}

void Appointment::makeEvents()
{
    // make an event list
    if( m_haveRecurrence )
    {
        connect( m_appRecurrence, SIGNAL(signalTick(int,int,int)),
              this, SIGNAL(sigTickEvent(int,int,int)) );

        qint64 seconds = m_appBasics->m_dtStart.secsTo( m_appBasics->m_dtEnd );
        QVector<DateTime> list = m_appRecurrence->recurrenceStartDates( m_appBasics->m_dtStart );

        qDebug() << "Appointment::makeEvents() " << list.count();


        for( const DateTime dt : list )
        {
            Event e;
            e.m_uid = m_appBasics->m_uid;
            e.m_displayText = m_appBasics->m_summary;
            e.m_startDt = dt;
            QDateTime qdt = dt.addSecs( seconds );
            e.m_endDt = DateTime( qdt.date(), qdt.time(), qdt.timeZone(), e.m_startDt.isDate() );
            e.m_isAlarmEvent = false;
            e.m_userCalendarId = m_userCalendarId;
            m_eventVector.append( e );
            m_yearsInQuestion.insert( dt.date().year() );
            m_yearsInQuestion.insert( qdt.date().year() );
            m_minYear = dt.date().year() < m_minYear ? dt.date().year() : m_minYear;
            m_maxYear = qdt.date().year() > m_maxYear ? qdt.date().year() : m_maxYear;

        }
        disconnect( m_appRecurrence, SIGNAL(signalTick(int,int,int)),
                 this, SIGNAL(sigTickEvent(int,int,int)) );
    }
    else
    {
        Event e;
        e.m_uid = m_appBasics->m_uid;
        e.m_displayText = m_appBasics->m_summary;
        e.m_startDt = m_appBasics->m_dtStart;
        e.m_endDt = m_appBasics->m_dtEnd;
        e.m_isAlarmEvent = false;
        e.m_userCalendarId = m_userCalendarId;
        m_eventVector.append( e );
        m_yearsInQuestion.insert( e.m_startDt.date().year() );
        m_yearsInQuestion.insert( e.m_endDt.date().year() );
        m_minYear = e.m_startDt.date().year();
        m_maxYear = e.m_endDt.date().year();
    }


    emit sigTickEvent( 0, 1, 1 );
}


void Appointment::setEventColor( const QColor inEventColor )
{
    for( Event &e : m_eventVector )
        e.m_eventColor = inEventColor;
}
