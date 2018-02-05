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
#ifndef APPOINTMENTMANAGER_H
#define APPOINTMENTMANAGER_H

#include "datetime.h"

#include <set>
#include <utility>

#include <QColor>
#include <QDebug>
#include <QSet>
#include <QString>
#include <QTimeZone>
#include <QVector>


/* AppointmentBasics is everything we need to show a single event in a calendar */
struct AppointmentBasics
{
    enum BusyFreeType {
        FREE,       // Appointment is nice to remember
        BUSY        // Appointment makes you busy, working, out of home
    };
    // === Methods ===
    AppointmentBasics();
    AppointmentBasics( const AppointmentBasics &other );

    bool operator==(const AppointmentBasics &other ) const;

    // For debugging purpose
    QString contententToString() const;

    // ==== Data ===
    QString         m_uid;
    int             m_sequence; // change mangement, every change to Appointment: m_sequence++;
    DateTime        m_dtStart;
    DateTime        m_dtEnd;
    QString         m_summary;
    QString         m_description;
    BusyFreeType    m_busyFree;
};


/* AppointmentAlarm reminds you for an event days or hours
 * before/after the event
 * The alarm is always relative to the start of the event,
 *  but can happen before and after the event.
 * As the alarm is always relative to DTSTART, no
 *  absolute/UTC DATETIMEis supported.
 *
 */
struct AppointmentAlarm
{
    // === Methods ===

    // Constructor: all dato set to 0
    AppointmentAlarm();
    AppointmentAlarm( const AppointmentAlarm &other );

    bool operator==(const AppointmentAlarm &other ) const;

    // for debugging
    QString contentToString() const;

    // generates a list of alarm times base on appointment start times.
    QVector<DateTime*> alarmTime( const QVector<DateTime> &inAppointmentTime );

    // === Data ===
    qint64  m_alarmSecs;        // first or only alarm in seconds rel. to DTSTART
    int     m_repeatNumber;     // number of repetitions after first alarm.
    quint64 m_pauseSecs;        // pause between two repetitions
};


class AppointmentRecurrence : public QObject
{

    Q_OBJECT

public:
    // === Types ===
    enum WeekDay {
        WD_MO = 1,  WD_TU = 2,  WD_WE = 3,  WD_TH = 4,
        WD_FR = 5,  WD_SA = 6,  WD_SU = 7
    };

    // the simple_ const show an every YEAR,.. without BY_XXX rules.
    enum RecurrenceFrequencyType {
        RFT_FIXED_DATES,                // list of fixed DateTimes
        RFT_SIMPLE_YEARLY,              // like birthdays
        RFT_SIMPLE_MONTHLY,
        RFT_SIMPLE_WEEKLY,              // every tuesday: GNU/Linux User Group Meeting
        RFT_SIMPLE_DAILY,
        RFT_YEARLY, RFT_MONTHLY,
        RFT_WEEKLY, RFT_DAILY
    };

    // === Methods ===

    AppointmentRecurrence( QObject *parent = Q_NULLPTR );

    // just partially, because we do not handle QObject
    void getAPartialCopy( const AppointmentRecurrence &other );
    bool isPartiallyEqual( const AppointmentRecurrence &other ) const;

    // for debugging
    QString     contentToString() const;

    // a list of dates, where the recurrence occurs
    // inDtStart is simply DTSTART,
    //  inDtLast is UNTIL - if UNTIL is valid - or a future date
    QVector<DateTime> recurrenceStartDates( const DateTime inDtStart );
    QVector<DateTime> recurrenceStartDatesSimpleYearly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesSimpleMonthly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesSimpleWeekly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesSimpleDaily( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesYearly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesMonthly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesWeekly( const DateTime inDtStart, const DateTime inDtLast );
    QVector<DateTime> recurrenceStartDatesDaily( const DateTime inDtStart, const DateTime inDtLast );

    /* weekExpand()
     * creates a list of dates in the given week.
     * a week starts with firstDayOfWeek()
     * the days are inside of DTSTART and Recurrence-UNTIL.
     * Every day in OutDaylist has time and timezone set by inRefDateTime
     */
    void        weekExpand( const DateTime inRefDateTime, const int inWeekNo, QVector<DateTime> &outDayList );

    /* timeExpand()
     * creates a list of times from hour, minutes and seconds list. Every missing value is
     *  filled up by inRefDateTime.
     */
    void        timeExpand(const DateTime inRefDateTime, const QSet<int> inHourSet, const QSet<int> inMinSet,
                           const QSet<int> inSecSet, QVector<QTime> &outTimeVector);

    /* firstDayOfWeek()
     * Tries to calculate the first day in the first week and then adds the number of
     * (weeks-1) * 7.
     * There are several rules to find the first day of the first
     * week of the year. If you are doing it right for one country, you are doing it
     * wrong for another country.
     * This implementation uses:
     *   minimum 4 days of the first week have to be in the time from 1st january.
     */
    QDate       firstDayOfWeek( const int inYear, const int inWeekNumber ) const;

    /* validateDateTime()
     * false, if m_recurrence.m_until is valid and inRefTime > UNTIL (too late)
     * else true
     * @fixme: Ping: think about EXDATEs, RDATEs
     */
    bool        validateDateTime( const DateTime inRefTime ) const ;

    // sorts the list in place.
    void        sortDaytimeList( QVector<DateTime> &inoutSortVector );

    bool daysetContainsDay( const std::set<std::pair<WeekDay, int>> inStdSet,
                            const WeekDay inWeekday ) const
    {
        for( const std::pair<WeekDay, int> dayElem : inStdSet )
        {
            if( dayElem.first == inWeekday )
                return true;
        }
        return false;
    }

    // === Data ===

    // for check, if we have received this (default: false)
    bool    m_haveCount;
    bool    m_haveUntil;

    RecurrenceFrequencyType     m_frequency;
    int                         m_count;
    int                         m_interval;
    DateTime                    m_until;
    WeekDay                     m_startWeekday;
    QVector<DateTime>           m_exceptionDates;
    QVector<DateTime>           m_fixedDates;
    QSet<int>                   m_byMonthSet;
    QSet<int>                   m_byWeekNumberSet;
    QSet<int>                   m_byYearDaySet;
    QSet<int>                   m_byMonthDaySet;
    std::set<std::pair<WeekDay, int>>   m_byDaySet;
    QSet<int>                   m_byHourSet;
    QSet<int>                   m_byMinuteSet;
    QSet<int>                   m_bySecondSet;
    QSet<int>                   m_bySetPosSet;

signals:
    void signalTick( int first, int current, int last );
};


struct Event {
    QString     m_uid;              // to find related Appointment
    QString     m_displayText;      // text to show in calendar
    DateTime    m_startDt;
    DateTime    m_endDt;
    bool        m_isAlarmEvent;     // true, if this is an alarm event
    int         m_userCalendarId;   // set by appointment
    QColor      m_eventColor;       // is set by appointment

    bool operator==(const Event other) const
    {
        return m_uid == other.m_uid and
                m_displayText == other.m_displayText and
                m_startDt == other.m_startDt and
                m_endDt == other.m_endDt and
                m_isAlarmEvent == other.m_isAlarmEvent;
    }

    bool operator<(const Event other) const
    {
        if( m_startDt.date() < other.m_startDt.date() )
            return true;
        if( m_startDt.date() > other.m_startDt.date() )
            return false;
        // equal and both are dates?
        if( m_startDt.isDate() and other.m_startDt.isDate() )
            return false;
        if( m_startDt.time() < other.m_startDt.time() )
            return true;
        if( m_startDt.time() > other.m_startDt.time() )
            return false;
        // both start DateTime-Values look equal and one of them is value-type datetime
        // compare end-times
        if( m_endDt.date() < other.m_endDt.date() )
            return true;
        if( m_endDt.date() > other.m_endDt.date() )
            return false;
        return false;
    }
    bool containsDay( const QDate inDate ) const
    {
        return m_startDt.date() <= inDate and m_endDt.date() >= inDate;
    }
    bool sameDay() const
    {
        return m_startDt.date() == m_endDt.date() ;
    }
};


class Appointment : public QObject
{

    Q_OBJECT

public:

    Appointment( QObject* parent = Q_NULLPTR );
    ~Appointment();

    /* this does only check SOME values stored here.
     * Especially, no sub-apoitments and nothing, a makeEvents() would generate */
    bool isPartiallyEqual( const Appointment &other ) const;

    // creates a possible unique UID
    void generateUid();

    // helper methods
    static void makeDateVector( const QString inElementsString, const QString inTimeZone, QVector<DateTime> &outVector );
    static void makeDayset( const QString inElementsString, std::set<std::pair<AppointmentRecurrence::WeekDay, int>> &outSet );
    static void makeIntSet( const QString inElementsString, QSet<int> &outSet );

    static void makeStringsFromDateVector( const QVector<DateTime> &inVector, QString &outDtString, QString &outTzString );
    static void makeStringFromDayset( const std::set<std::pair<AppointmentRecurrence::WeekDay, int>> inSet, QString &outString );
    static void makeStringFromIntSet( const QSet<int> inIntSet, QString &outString );

    void makeEvents();
    void setEventColor( const QColor inEventColor );

    AppointmentBasics*          m_appBasics;
    AppointmentRecurrence*      m_appRecurrence;
    QVector<AppointmentAlarm*>  m_appAlarms;
    // years
    QSet<int>                   m_yearsInQuestion;  // all the years
    int                         m_minYear;          // start of first event
    int                         m_maxYear;          // end of last event
    // events
    QVector<Event>              m_eventVector;
    // calendar id
    int                         m_userCalendarId;
    QString                     m_uid;
    // getter
    bool                        m_haveRecurrence;
    bool                        m_haveAlarm;

private:
    //Appointment( const Appointment & );


signals:
    void sigTickEvent( const int min, const int current, const int max );
};


#endif // APPOINTMENTMANAGER_H
