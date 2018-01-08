/*  Daylight - day planner, appointment book application
    Copyright (C) 2014  Eike Lange (eike(at)ngc42.de)

    Daylight is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef DAYITEM_H
#define DAYITEM_H

#include <QGraphicsObject>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSimpleTextItem>
#include <QKeyEvent>
#include <QPainter>
#include <QMenu>
#include <QAction>
#include <QDate>
#include "appointment.h"



/* Every DayItem has a visible sign to show, that there are more appointments than
 * space to display them. Show/hide and position  are controlled by the owner.*/
class TooManyAppointmentsItem : public QGraphicsItem
{
public:
    explicit TooManyAppointmentsItem(QGraphicsItem* parent = 0);
    QRectF boundingRect() const { return QRectF({0, 0}, m_size); }
    void resize(const qreal width, const qreal height);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);

private:
    QSizeF m_size;
};



/* AppointmentItems are the visual representations of of Appointments.
 * Dummy items are hidden items without any feedback, just there to fill some space as a DayItem may
 * need for the concept of slots.
 * Normal AppointmentItems show a tool tip, a context menu (delete and reconfigure), a title, and a
 * rectangular colored area, representing the duration.
 * If the size of the appointment within a DayItem is too small, then nothing is shown.
 * deleting the appointment means deleting this appointment item. This item may get
 *   deleted during execution of the signal handler. We use here a call to
 *   QMetaObject::invokeMethod(this, "signalDeleteAppointment", Qt::QueuedConnection, Q_ARG(int, m_appointmentId))
 *   in slotPrepareDeleteAppointment() and a "delayed connect" in MainWindow (Qt::QueuedConnection).
 */
class AppointmentItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit AppointmentItem(QGraphicsItem* parent = 0);    // dummy Item
    explicit AppointmentItem(Appointment* appointment, QGraphicsItem* parent = 0);
    ~AppointmentItem();
    bool dummy() const { return m_dummy; }
    QRectF boundingRect() const { return QRectF({0, 0}, m_size); }
    QDateTime startDt() const { return m_startDt; }
    QDateTime endDt() const { return m_endDt; }
    void setFontPixelSize(int size);
    void resize(const qreal width, const qreal height);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QString title() const { return m_title; }
    void setShowTitle(const bool show);

private:
    bool m_dummy;
    QSizeF m_size;
    bool m_sizeTooSmall;       // true, if size too small, then we don't paint anything
    QColor m_color;
    QString m_title;
    bool m_showTitle;
    int m_fontPixelSize;
    // copy from appointment
    int m_userCalendarId;
    int m_appointmentId;
    QDateTime m_startDt, m_endDt;
    bool m_allDay;
    // context menu
    QMenu m_contextMenu;
    QAction* m_actionReconfigureAppointment;
    QAction* m_deleteThisAppointment;

protected:
    void keyReleaseEvent(QKeyEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent* event);
    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event);

signals:
    void signalReconfigureAppointment(int apointmentId);
    void signalDeleteAppointment(const int appointmentId);

private slots:
    void slotPrepareReconfigureAppointment();
    void slotPrepareDeleteAppointment();
};



/* DayItem is the base class of all items representing days.
 * DayItems emits a signal on double click and forwards appointment
 * signals to the CalendarScene.
 * Setting the right font for the given item size and display a day number
 * label is a main part of DayItem. */
class DayItem : public QGraphicsObject
{
    Q_OBJECT

public:
    explicit DayItem(QGraphicsItem *parent = 0);
    ~DayItem();
    QRectF boundingRect() const { return QRectF({0, 0}, m_size); }
    void resize(const qreal width, const qreal height);
    int perfectFontSizeForString(const QString & text, const int width, const int height) const;
    void setFontPixelSize(const int size);
    virtual void setDate(const QDate date);
    QDate date() const { return m_displayDate; }
    void setDayText(const QString dayString);

private:
    QDate m_displayDate;

protected:
    QGraphicsSimpleTextItem* m_dayLabel;
    QSizeF m_size;
    void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event);

signals:
    void signalReconfigureAppointment(int apointmentId);
    void signalDeleteAppointment(int apointmentId);
    void signalDateClicked(const QDate & date);


public slots:
    void slotDeleteAppointment(const int _id);

};



/* A Day within the year view. This is normally a very small day representation,
 * except for really big screens. A DayInYearItem manages appointments, a sign
 * for too many appointments and a label for week numbers.
 * There are 2 different appointment lists: one for longer than a day appointments (range),
 * and one for in-day appointments, wich are shown more on the right side of the day.
 * There are slots for range appointments. each slot represents a straight vertical line in the
 * calendar. Gaps between 2 slots are filled with dummy appointment items. */
class DayInYearItem : public DayItem
{
    Q_OBJECT

public:
    explicit DayInYearItem(const QDate date, QGraphicsItem* parent = 0);
    ~DayInYearItem();
    void resize(const qreal width, const qreal height);
    void setFontPixelSize(const int size);
    void adjustSubitemPositions();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void setDate(const QDate date);

    void setAppointmentDaySlots(const QList<Appointment*> & list);
    void setAppointmentRangeSlot(const int slot, const QList<Appointment*> & list);
    void clearAppointments();

private:
    QGraphicsSimpleTextItem* m_weekNumberLabel;
    TooManyAppointmentsItem* m_tooManyItems;
    QList<AppointmentItem*> m_appointmentSlotsDay;
    QList<AppointmentItem*> m_appointmentSlotsRange;

signals:
public slots:
};



/* represents a day in a month or in 3-weeks view.
 * AppointmentItems are layouted horizontally. */
class DayInMonthItem : public DayItem
{
    Q_OBJECT

public:
    explicit DayInMonthItem(const QDate date, QGraphicsItem* parent = 0);
    ~DayInMonthItem();
    void resize(const qreal width, const qreal height);
    void adjustSubitemPositions();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void setDate(const QDate date);

    void setAppointmentRangeSlot(const int slot, const QList<Appointment*> & list, int weekStart);
    void setAppointments(const QList<Appointment*> & list);
    void clearAppointments();

private:
    TooManyAppointmentsItem* m_tooManyItems;
    QList<AppointmentItem*> m_appointmentSlots;

signals:

public slots:
};



/* represents a day in a week view.
 * This is somewhat special, as further development may provide time markers, similar to
 * DayInDayItem. */
class DayInWeekItem : public DayItem
{
    Q_OBJECT

public:
    explicit DayInWeekItem(const QDate date, QGraphicsItem* parent = 0);
    ~DayInWeekItem();
    void resize(const qreal width, const qreal height);
    void adjustSubitemPositions();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void setDate(const QDate date);

    void setAppointmentRangeSlot(const int slot, const QList<Appointment*> & list, int weekStart);
    void setAppointments(const QList<Appointment*> & list);
    void clearAppointments();

private:
    TooManyAppointmentsItem* m_tooManyItems;
    QList<AppointmentItem*> m_appointmentSlots;

signals:
public slots:
};



/* This one is just a day. Shown with Time. The Appointment-Slots are much easier,
 * as there is no gap between 2 of them.
 * DayInDayItem has markers for hours. The active hours of a day are set by constructor and setActiveDaytime().
 * Markers are calculated in createMarkerList() and displayed during a paint. */
class DayInDayItem : public DayItem
{
    Q_OBJECT

private:
    // positions for hour markers and time areas
    struct Marker
    {
        int ypos;
        // -2: full day area,
        // -1: hours before DayInDayItem::m_activeHourStart (if available),
        // 25: hours after DayInDayItem::m_activeHourEnd
        int hour;
    };
    int m_deltaPixelForHour;    // one hour has this for height


public:
    explicit DayInDayItem(const QDate date, QGraphicsItem* parent = 0);
    ~DayInDayItem();
    void resize(const qreal width, const qreal height);
    void adjustSubitemPositions();
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    virtual void setDate(const QDate date);
    void setActiveDaytime(int hourBegin, int hourEnd);

    void setAppointmentsFullDay(const QList<Appointment*> & list);
    void setAppointmentsPartDay(const QList<Appointment*> & list);
    void clearAppointments();

private:
    int m_activeHourStart;      // time to display
    int m_activeHourEnd;
    TooManyAppointmentsItem* m_tooManyItems;
    QList<AppointmentItem*> m_appointmentFullDay;
    QList<AppointmentItem*> m_appointmentPartDay;
    int m_numberOfAppointmentOverlaps;
    QList<Marker> m_markerList;
    void createMarkerList();

signals:

public slots:
};



#endif // DAYITEM_H