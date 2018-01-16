#ifndef APPOINTMENTMANAGER_H
#define APPOINTMENTMANAGER_H

#include <QList>
#include <QMultiMap>
#include <QSet>
#include <QString>
#include <QTimeZone>
#include <QVector>

#include "datetime.h"


/* AppointmentBasics is everything we need to show a single event in a calendar */
struct AppointmentBasics
{
    enum BusyFreeType {
        FREE,       // Appointment is nice to remember
        BUSY        // Appointment makes you busy, working, out of home
    };
    // === Methods ===
    AppointmentBasics();

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

    // for debugging
    QString contentToString() const;

    // generates a list of alarm times base on appointment start times.
    QList<DateTime*> alarmTime( const QList<DateTime> &inAppointmentTime );

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
    void        timeExpand(const DateTime inRefDateTime, const QList<int> inHourList, const QList<int> inMinList,
                           const QList<int> inSecList, QVector<QTime> &outTimeList);

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
    void        sortDaytimeList( QVector<DateTime> &inoutSortList );

    // === Data ===

    RecurrenceFrequencyType     m_frequency;
    int                         m_count;
    int                         m_interval;
    DateTime                    m_until;
    WeekDay                     m_startWeekday;
    QList<DateTime>             m_exceptionDates;
    QList<DateTime>             m_fixedDates;
    QList<int>                  m_byMonthList;
    QList<int>                  m_byWeekNumberList;
    QList<int>                  m_byYearDayList;
    QList<int>                  m_byMonthDayList;
    QMultiMap<WeekDay, int>     m_byDayMap;
    QList<int>                  m_byHourList;
    QList<int>                  m_byMinuteList;
    QList<int>                  m_bySecondList;
    QList<int>                  m_bySetPosList;

signals:
    void signalTick( int first, int current, int last );
};


struct Event {
    QString     m_uid;              // to find related Appointment
    QString     m_displayText;      // text to show in calendar
    DateTime    m_startDt;
    DateTime    m_endDt;
    bool        m_isAlarmEvent;     // true, if this is an alarm event
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
};


class Appointment : public QObject
{

    Q_OBJECT

public:

    Appointment( QObject *parent = Q_NULLPTR );

    // helper methods
    static void makeDateList( const QString inElementsString, const QString inTimeZone, QList<DateTime> &outList );
    static void makeDaymap( const QString inElementsString, QMultiMap<AppointmentRecurrence::WeekDay, int> &outMap );
    static void makeIntList( const QString inElementsString, QList<int> &outList );
    static void makeStringsFromDateList( const QList<DateTime> &inList, QString &outDtString, QString &outTzString );
    static void makeStringFromDaymap( const QMultiMap<AppointmentRecurrence::WeekDay, int> inMap, QString &outString );
    static void makeStringFromIntList( const QList<int> inIntList, QString &outString );
    static void makeStringFromIntSet( const QSet<int> inIntSet, QString &outString );

    void makeEvents();

    AppointmentBasics*          m_appBasics;
    AppointmentRecurrence*      m_appRecurrence;
    QList<AppointmentAlarm*>    m_appAlarms;
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
    Appointment( const Appointment & );


signals:
    void sigTickEvent( const int min, const int current, const int max );
};


#endif // APPOINTMENTMANAGER_H
