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
#include "appointment.h"
#include <QDebug>


/***********************************************************
********** Appointment *************************************
***********************************************************/


Appointment::Appointment(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & colorCopy)
{
    modifyAppointment(apmData, recData, colorCopy);
}


void Appointment::modifyAppointment(const AppointmentData & newApmData, const RecurrenceData & newRecData, const QColor & newColor)
{
    m_appointmentData = newApmData;
    m_recurrenceData = newRecData;
    m_color = newColor;
    m_sameDay = m_appointmentData.m_startDt.date() == m_appointmentData.m_endDt.date();
    // this is not true for some appointments containing 29. feb.
    m_numAffectedDays = m_appointmentData.m_startDt.date().daysTo(m_appointmentData.m_endDt.date()) + 1;
}


bool Appointment::sortsBefore(Appointment* & other) const
{
    if(startDateTime().date() < other->startDateTime().date() )
        return true;
    if(this->sortsSame(other))
        return startDateTime() < other->startDateTime();
    if(startDateTime().date() == other->startDateTime().date())
        return ((numAffectedDays() > other->numAffectedDays()) or
            ((numAffectedDays() == other->numAffectedDays()) and
             (m_appointmentData.m_allDay > other->m_appointmentData.m_allDay)));
    return false;
}


bool Appointment::sortsSame(Appointment* & other) const
{
    // same day, same range and same allDay-status
    return ((startDateTime().date() == other->startDateTime().date()) and
            (numAffectedDays() == other->numAffectedDays()) and
            (m_appointmentData.m_allDay == other->m_appointmentData.m_allDay) );
}


bool Appointment::containsDay(const QDate & d) const
{
    if(m_appointmentData.m_startDt.date() > d)
        return false;

    switch(m_recurrenceData.m_type)
    {
        case R_NO_RECURRENCE:
            return m_appointmentData.m_endDt.date() >= d;
        case R_YEAR:
        {
            if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < d)
                return false;

            QDate dbegin = m_appointmentData.m_startDt.date();
            QDate dend = m_appointmentData.m_endDt.date();
            int diffYear = dend.year() - dbegin.year();
            if(diffYear == 0)
            {
                dbegin.setDate(d.year(), dbegin.month(), dbegin.day());
                dend.setDate(d.year(), dend.month(), dend.day());
                return (d >= dbegin and d <= dend);
            }
            // else: year differs
            // 1. start with begin-date = current year
            dbegin.setDate(d.year(), dbegin.month(), dbegin.day());
            dend.setDate(d.year() + diffYear, dend.month(), dend.day());
            if (d >= dbegin and d <= dend)
                return true;
            // 2. test: end date is current year
            dend.setDate(d.year(), dend.month(), dend.day());
            dbegin.setDate(d.year()-diffYear, dbegin.month(), dbegin.day());
            return (d >= dbegin and d <= dend);
        }
            break;
        case R_MONTH:
        {
            if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < d)
                return false;
            QDate dbegin = m_appointmentData.m_startDt.date();
            dbegin.setDate(d.year(), d.month(), dbegin.day());
            QDate dend = dbegin.addDays(m_appointmentData.m_startDt.date().daysTo(m_appointmentData.m_endDt.date()));
            return (d >= dbegin and d <= dend);
        }
            break;
        case R_WEEK:
        {
            if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < d)
                return false;
            QDate dbegin = d.addDays(d.dayOfWeek() - m_appointmentData.m_startDt.date().dayOfWeek());
            QDate dend = dbegin.addDays(m_appointmentData.m_startDt.date().daysTo(m_appointmentData.m_endDt.date()));
            return (d >= dbegin and d <= dend);
        }
            break;
        case R_DAY:
            return (m_recurrenceData.m_forever or m_recurrenceData.m_lastDt.date() >= d);
            break;
    }
    return false;
}


bool Appointment::containsRange(const QDate & start, const QDate & end) const
{
    if(m_appointmentData.m_startDt.date() > end)
        return false;
    if(m_recurrenceData.m_type == R_NO_RECURRENCE)
        return m_appointmentData.m_startDt.date() <= end and m_appointmentData.m_endDt.date() >= start;
    if(m_recurrenceData.m_type == R_YEAR)
    {
        if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < start)
            return false;
        QDate dbegin = m_appointmentData.m_startDt.date();
        QDate dend = m_appointmentData.m_endDt.date();
        int diffYear = dend.year() - dbegin.year();
        if(diffYear == 0)
        {
            dbegin.setDate(start.year(), dbegin.month(), dbegin.day());
            dend.setDate(start.year(), dend.month(), dend.day());
            return (end >= dbegin and start <= dend);
        }
        // else: year differs
        // 1. start with begin-date = start year
        dbegin.setDate(start.year(), dbegin.month(), dbegin.day());
        dend.setDate(start.year() + diffYear, dend.month(), dend.day());
        if (end >= dbegin and start <= dend)
            return true;
        // 2. test: end date is current year
        dend.setDate(start.year(), dend.month(), dend.day());
        dbegin.setDate(start.year()-diffYear, dbegin.month(), dbegin.day());
        return (end >= dbegin and start <= dend);


    }
    if(m_recurrenceData.m_type == R_MONTH)
    {
        if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < start)
            return false;
        QDate dbegin = m_appointmentData.m_startDt.date();
        dbegin.setDate(start.year(), start.month(), dbegin.day());
        QDate dend = m_appointmentData.m_endDt.date();
        dend.setDate(end.year(), end.month(), dend.day());
        return (dbegin <= end and dend >= start);
    }
    if(m_recurrenceData.m_type == R_WEEK)
    {
        if(!m_recurrenceData.m_forever and m_recurrenceData.m_lastDt.date() < start)
        {
            return false;
        }
        // except for days, which are handled by Appointment::containsDay(),
        // this is always true as start...end contains at least one week
        return true;
    }
    if(m_recurrenceData.m_type == R_DAY)
        return (m_recurrenceData.m_forever or m_recurrenceData.m_lastDt.date() >= start);
    return false;
}



/***********************************************************
********** AppointmentPool *********************************
***********************************************************/

AppointmentPool::AppointmentPool()
{
    m_nextAppointmentId = 10;
}


QList<AppointmentData> AppointmentPool::appointmentData() const
{
    QList<AppointmentData> dataList;
    for(Appointment* apm : m_pool)
    {
        dataList.append(apm->m_appointmentData);
    }
    return dataList;
}


/* return insert id */
int AppointmentPool::add(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color)
{
    AppointmentData newApmData = apmData;
    RecurrenceData newRecData = recData;
    if(apmData.m_appointmentId == 0)
    {
        newApmData.m_appointmentId = m_nextAppointmentId;
        newRecData.m_appointmentId = m_nextAppointmentId;
        m_nextAppointmentId++;
    }
    Appointment* apm = new Appointment(newApmData, newRecData, color);
    add(apm);
    return newApmData.m_appointmentId;
}


// private
void AppointmentPool::add(Appointment* & anAppointment)
{
    int numItems = m_pool.count();
    if(numItems == 0 or m_pool[numItems - 1]->sortsBefore(anAppointment))
        m_pool.append(anAppointment);
    else if(anAppointment->sortsBefore(m_pool[0]))
        m_pool.prepend(anAppointment);
    else
    {
        int idxStart = 0;
        int idxEnd = numItems;
        int idxMid = (idxStart + idxEnd) / 2;

        while( (idxEnd - idxStart) > 0 )
        {
            if(m_pool[idxMid]->sortsBefore(anAppointment))
                idxStart = idxMid + 1;
            else if(anAppointment->sortsBefore(m_pool[idxMid]))
                idxEnd = idxMid;
            else // eqal
            {
                if(m_pool[idxMid]->numAffectedDays() > anAppointment->numAffectedDays())
                    idxStart = idxMid;
                else
                    idxEnd = idxMid;
            }
            idxMid = (idxStart + idxEnd) / 2;
        }
        m_pool.insert(idxMid, anAppointment);
    }
}


void AppointmentPool::modifyAppointment(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color)
{
    for(Appointment* & apm : m_pool)
    {
        if(apm->m_appointmentData.m_appointmentId == apmData.m_appointmentId)
        {
            apm->modifyAppointment(apmData, recData, color);
            break;
        }
    }
}


void AppointmentPool::addAppointmentFromStorage(const AppointmentData & apmData, const RecurrenceData & recData, const QColor & color)
{
    Appointment* apm = new Appointment(apmData, recData, color);
    m_nextAppointmentId = apmData.m_appointmentId >= m_nextAppointmentId ? apmData.m_appointmentId + 1 : m_nextAppointmentId;
    add(apm);
}


/* remove 0..1 Appointment with given appointment id. */
void AppointmentPool::removeById(const int appointmentId)
{
    for(Appointment* apm : m_pool)
    {
        if(apm->m_appointmentData.m_appointmentId == appointmentId)
        {
            m_pool.removeOne(apm);
            delete apm;
            break;
        }
    }
}


/* This method is called whenever main program wants to delete a calendar. We delete all
 * the appointments in the pool.
 * This method does not emit any signals. */
void AppointmentPool::removeByCalendarId(const int calendarId)
{
    for(Appointment* apm : m_pool)
    {
        if(apm->m_appointmentData.m_userCalendarId == calendarId)
        {
            m_pool.removeOne(apm);
            delete apm;
        }
    }
}

void AppointmentPool::setColorForId(const int userCalendarId, const QColor & color)
{
    for(Appointment* a : m_pool)
    {
        if(a->m_appointmentData.m_userCalendarId == userCalendarId)
        {
            a->m_color = color;
        }
    }
}


void AppointmentPool::setAppointmentCalendarForId(const int appointmentId, const int newCalendarId, const QColor & calendarColor)
{
    for(Appointment* a : m_pool)
    {
        if(a->m_appointmentData.m_appointmentId == appointmentId)
        {
            a->m_appointmentData.m_userCalendarId = newCalendarId;
            a->m_color = calendarColor;
            break;
        }
    }
}

const QList<Appointment*> AppointmentPool::appointmentForRange(const QDate & start, const QDate & end) const
{
    QList<Appointment*> daylist;
    for(Appointment* apm : m_pool)
    {
        if(apm->containsRange(start, end))
            daylist.append(apm);

    }
    return daylist;
}


const QList<Appointment*> AppointmentPool::appointmentForDay(const QDate & day) const
{
    QList<Appointment*> daylist;
    for(Appointment* apm : m_pool)
    {
        if(apm->containsDay(day))
            daylist.append(apm);
    }
    return daylist;
}


const QList<Appointment*> AppointmentPool::appointmentForWeek(const QDate & day, int weekStart) const
{
    QDate start(day), end;
    int dayDelta = weekStart - day.dayOfWeek();
    if (dayDelta > 0)
        dayDelta = dayDelta - 7;
    start = start.addDays(dayDelta);
    end = start.addDays(6);
    return appointmentForRange(start, end);
}


const QList<Appointment*> AppointmentPool::appointmentFor3Weeks(const QDate & day, int weekStart) const
{
    QDate start(day), end;
    int dayDelta = weekStart - day.dayOfWeek();
    if (dayDelta > 0)
        dayDelta = dayDelta - 7;
    start = start.addDays(dayDelta);
    start = start.addDays(-7);                      // - 1 week
    end = start.addDays(20);
    return appointmentForRange(start, end);
}


const QList<Appointment*> AppointmentPool::appointmentForMonth(const QDate & day, int weekStart) const
{
 //   QList<Appointment*> daylist;
    QDate start, end;
    start.setDate(day.year(), day.month(), 1);
    start = start.addDays(weekStart - start.dayOfWeek());
    end = start.addDays(6 * 7 - 1);
    return appointmentForRange(start, end);
}


const QList<Appointment*> AppointmentPool::appointmentForYear(const QDate & day) const
{
    QDate start, end;
    start.setDate(day.year(), 1, 1);
    end.setDate(day.year(), 12, 31);
    return appointmentForRange(start, end);
}


Appointment* AppointmentPool::appointmentById(const int id) const
{
    for(Appointment* apm : m_pool)
    {
        if(apm->m_appointmentData.m_appointmentId == id)
            return apm;
    }
    return nullptr;
}

