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
#ifndef CALENDARSCENE_H
#define CALENDARSCENE_H

#include <QGraphicsScene>

#include "appointmentmanager.h"
#include "calendarheader.h"
#include "dayitem.h"
#include "settingsdialog.h"


/* An enum to describe, what is actually shown in CalendarScene */
enum class CalendarShow {SHOW_UNKNOWN, SHOW_YEAR, SHOW_MONTH, SHOW_3WEEKS, SHOW_WEEK, SHOW_DAY};


/* CalendarScene is created in MainWindow. The purpose is to display the various ways of Calendars
 * with their days, which contain the appointments.
 * CalendarScene creates all the dates for every view and holds the current selected date.
 * The current show is started by SettingsData::m_last_view in main window and controlled by menu/toolbar
 * actions over slots like slotShowYear() and friends.
 */
class CalendarScene : public QGraphicsScene
{
    Q_OBJECT

public:
    explicit CalendarScene( const SettingsData & settings, QObject* parent = Q_NULLPTR );
    ~CalendarScene();

    void setDate(const QDate & date, bool update = false);
    QDate date() const { return m_currentBaseDate; }
    CalendarShow showView() const { return m_showView; }
    void updateSize(const QSize & newSize);

    void setEventsForYear(const QVector<Event>& list);
    void setEventsForMonth(const QVector<Event> &list);

    void setAppointmentsFor3Weeks(const QVector<Event> &list);
    void setAppointmentsForWeek(const QVector<Event> &list);
    void setAppointmentsForDay(const QVector<Event> &list);
    void removeAllEvents();
    void removeEventsById( const QString appointmentId );
    void setSettings(const SettingsData & settings);

    void eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor );

private:
    // date
    QDate m_currentBaseDate;
    int m_weekStartDay;         // 1 = Mon, ..., 7 = Sun

    // header items
    void createHeader();
    QVector<HeaderMonth*>       m_headersMonth;
    QVector<HeaderWeekday*>     m_headersWeekday;
    QString                     m_longestMonthName;
    QString                     m_longestDayName;

    // day items
    void createDays();
    QVector<DayInYearItem*> m_daysInYearItems;
    QVector<DayInMonthItem*> m_daysInMonthItems;
    QVector<DayInMonthItem*> m_daysIn3WeeksItems;
    QVector<DayInWeekItem*> m_daysInWeekItems;
    DayInDayItem* m_dayInDayItem;

    QString m_longestDaylabelTextInYear;

    void hideAllItems();

    CalendarShow m_showView;

signals:
    void signalReconfigureAppointment(QString appointmentId);
    void signalDeleteAppointment(QString appointmentId);
    void signalDateClicked(const QDate & date);

public slots:
    void slotShowYear();
    void slotShowMonth();
    void slotShow3Weeks();
    void slotShowWeek();
    void slotShowDay();
};

#endif // CALENDARSCENE_H
