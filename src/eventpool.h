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
    void addAppointment(Appointment* inApp );
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
    void changeColor(const int inUserCalendarId, const QColor inNewColor);


    // Events
    QVector<Event> eventsByYear( const int inYear ) const;
    QVector<Event> eventsByYearMonth( const int inYear, const int inMonth ) const;
    QVector<Event> eventsByYearMonthDay( const int inYear, const int inMonth, const int inDay ) const;
    QVector<Event> eventsByYearMonthDayRange( const int inYearS, const int inMonthS, const int inDayS,
                                              const int inYearE, const int inMonthE, const int inDayE) const;

private:
    QVector<Appointment*>       m_appointments;

    // set of uid, just to mark which ones we have, no duplicates.
    QSet<QString>               m_appointmentsRead;

    // set of years to make update easier, see above
    QSet<int>                   m_yearMarkers;

    QMap<int, QVector<Event>>   m_eventMap; // year and event list

};

#endif // EVENTPOOL_H
