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
#include "calendarscene.h"
#include <QDebug>



CalendarScene::CalendarScene(const SettingsData & settings, QObject *parent) :
    QGraphicsScene(parent),
    m_currentBaseDate(QDate::currentDate()),
    m_weekStartDay(1),
    m_showView(CalendarShow::SHOW_UNKNOWN)
{
    setBackgroundBrush(Qt::white);

    // longest month name
    QString name;
    QFontMetrics mMetrics = QFontMetrics(font());
    int maxWidth = 0, width = 0;
    for(int month = 1; month <= 12; month++)
    {
        name =QDate(2000,month, 1).toString( "MMMM");
        width = mMetrics.width(name);
        if(width > maxWidth)
        {
            maxWidth = width;
            m_longestMonthName = name;
        }
    }
    // longest day name
    maxWidth = 0;
    for(int day = 1; day <= 7; day++)
    {
        name = QDate(2018, 1, day).toString( "dddd" );
        width = mMetrics.width(name);
        if(width > maxWidth)
        {
            maxWidth = width;
            m_longestDayName = name;
        }
    }
    // this should be the longest label text for a m_dayLabel in DayInYearItem (DayItem)
    m_longestDaylabelTextInYear = "38 MM.";

    createHeader();
    createDays();
    setSettings(settings);
}


void CalendarScene::setDate(const QDate & date, bool update)
{
    if((date == m_currentBaseDate) and (! update))
        return;

    int deltaDays;

    // Year items
    if(date.year() != m_currentBaseDate.year())
    {
        for(DayInYearItem* itm : m_daysInYearItems)
        {
            QDate d = itm->date();
            d.setDate(date.year(), d.month(), d.day());
            if(! d.isValid())
            {
                itm->hide();
                if(QDate::isLeapYear(date.year()))
                {
                    d.setDate(date.year(), 2, 29);
                    // show 29. feb. on LeapYears and if other items are shown too
                    itm->setVisible(m_daysInYearItems[0]->isVisible());
                }
            }
            itm->setDate(d);
            itm->update();
        }
    }

    // Month items
    QDate d(date);
    d.setDate(d.year(), d.month(), 1);
    deltaDays = m_weekStartDay - d.dayOfWeek();
    deltaDays = deltaDays > 0 ? deltaDays - 7 : deltaDays;
    d = d.addDays(deltaDays);
    for(DayInMonthItem* itm : m_daysInMonthItems)
    {
        QDate idate = itm->date();
        idate.setDate(d.year(), d.month(), d.day());
        itm->setDate(idate);
        itm->update();
        d = d.addDays(1);
    }


    // 3 Weeks items
    d.setDate(date.year(), date.month(), date.day());
    deltaDays = m_weekStartDay - d.dayOfWeek();
    deltaDays = deltaDays > 0 ? deltaDays - 7 : deltaDays;
    d = d.addDays(deltaDays);
    d = d.addDays(-7);
    for(DayInMonthItem* itm : m_daysIn3WeeksItems)
    {
        itm->setDate(d);
        itm->update();
        d = d.addDays(1);
    }

    // Week items
    d.setDate(date.year(), date.month(), date.day());
    deltaDays = m_weekStartDay - d.dayOfWeek();
    deltaDays = deltaDays > 0 ? deltaDays - 7 : deltaDays;
    d = d.addDays(deltaDays);
    for(DayInWeekItem* itm : m_daysInWeekItems)
    {
        itm->setDate(d);
        itm->update();
        d = d.addDays(1);
    }

    // single day item
    m_dayInDayItem->setDate(date);
    m_dayInDayItem->update();

    m_currentBaseDate = date;
}


void CalendarScene::updateSize(const QSize & newSize)
{
    // some boring calculations about width and height of items and header
    qreal newWidth = newSize.width();
    qreal newHeight = newSize.height();

    setSceneRect(0, 0, newWidth, newHeight);

    qreal width7 = newWidth / 7.0f;
    qreal width12 = newWidth / 12.0f;

    qreal headerHeight = 20.0f;
    qreal height32 = (newHeight - headerHeight) / 31.0f;
    qreal height6 = (newHeight - headerHeight) / 6;
    qreal height3 = (newHeight - headerHeight) / 3;
    qreal heightWeekItem = newHeight - headerHeight;

    // update header items
    int num = 0;
    int pixelSize = m_headersWeekday[0]->perfectFontSizeForString(m_longestDayName, width7, headerHeight);
    for(HeaderWeekday* h : m_headersWeekday)
    {
        if(pixelSize > 8)
        {
            h->setFontPixelSize(pixelSize);
            h->setShowLong(true);
        }
        else
        {
            h->setFontPixelSize(pixelSize * 2);
            h->setShowLong(false);
        }
        h->resize(width7, headerHeight);
        h->setPos(width7 * num, 0.0f);
        update();
        num++;
    }

    num = 0;
    pixelSize = m_headersMonth[0]->perfectFontSizeForString(m_longestMonthName, width12, headerHeight);
    for(HeaderMonth* h : m_headersMonth)
    {
        if(pixelSize > 8)
        {
            h->setFontPixelSize(pixelSize);
            h->setShowLong(true);
        }
        else
        {
            h->setFontPixelSize(pixelSize * 2);
            h->setShowLong(false);
        }
        h->resize(width12, headerHeight);
        h->setPos(width12 * num, 0.0f);
        num++;
    }

    // days in year items
    qreal x = 0;
    qreal y = 0;
    pixelSize = m_daysInYearItems[0]->perfectFontSizeForString(m_longestDaylabelTextInYear, width12 / 3, height32 / 2);
    for(DayInYearItem* d : m_daysInYearItems)
    {
        d->resize(width12, height32);
        d->setFontPixelSize(pixelSize + 1);
        QDate day = d->date();
        if( ! day.isValid() )
        {
            x = width12;
            y = 28 * height32 + headerHeight;
        }
        else
        {
            x = (day.month() - 1) * width12;
            y = (day.day() - 1) * height32 + headerHeight;
        }
        d->setPos(x, y);
    }

    // days in month items
    x = 0, y = headerHeight;
    for(DayInMonthItem* d : m_daysInMonthItems)
    {
        d->resize(width7, height6);
        d->setPos(x * width7, y);
        x++;
        if(x == 7)
        {
            x = 0;
            y += height6;
        }
    }

    // days in month items
    x = 0, y = headerHeight;
    for(DayInMonthItem* d : m_daysIn3WeeksItems)
    {
        d->resize(width7, height3);
        d->setPos(x * width7, y);
        x++;
        if(x == 7)
        {
            x = 0;
            y += height3;
        }
    }

    // days in a week items
    x = 0, y = headerHeight;
    for(DayInWeekItem* d : m_daysInWeekItems)
    {
        d->resize(width7, heightWeekItem);
        d->setPos(x * width7, y);
        x++;
    }

    m_dayInDayItem->resize(newWidth, newHeight);
    m_dayInDayItem->setPos(0, 0);
}


void CalendarScene::setAppointmentsForYear( const QVector<Event> &list )
{
    for(DayInYearItem* d : m_daysInYearItems)
        d->clearAppointments();
    if(list.isEmpty())
        return;
    QVector<Event> currentList;
    QVector<Event> nextList;
    QVector<Event> dayList;

    // dispatch all appointments to a day and a range list
    for(Event e : list)
    {
        if( e.sameDay() )
        {
            dayList.append(e);
            qDebug() << "CalendarScene::setAppointmentsForYear - appended";
        }
        else
            nextList.append(e);
    }

    // day items to the day
    if(! dayList.isEmpty())
        for(DayInYearItem* d : m_daysInYearItems)
            d->setAppointmentDaySlots(dayList);

    // range items
    int slotNum = 0;
    QDate endDate;
    while( ! nextList.isEmpty())
    {
        currentList.append(nextList[0]);
        endDate = nextList[0].m_endDt.date();
        nextList.removeAt(0);

        for(const Event e : nextList)
        {
            if(e.m_startDt.date() > endDate)
            {
                currentList.append(e);
                endDate = e.m_endDt.date();
                nextList.removeOne(e);
            }
        }

        for(DayInYearItem* d : m_daysInYearItems)
        {
            d->setAppointmentRangeSlot(slotNum, currentList);
        }
        currentList.clear();
        slotNum++;
    }
}


void CalendarScene::setAppointmentsForMonth(const QVector<Event> &list)
{
    for(DayInMonthItem* d : m_daysInMonthItems)
        d->clearAppointments();
    if(list.isEmpty())
        return;
    QVector<Event> currentList;
    QVector<Event> nextList;
    QVector<Event> dayList;
    QVector<Event> allDayList;

    // dispatch all appointments to day and a range lists
    for(Event e : list)
    {
        if(e.sameDay())
        {
            dayList.append(e);
        }
        else
            nextList.append(e);
    }

    // range items
    int slotNum = 0;
    QDate endDate;
    while( ! nextList.isEmpty())
    {
        currentList.append(nextList[0]);
        endDate = nextList[0].m_endDt.date();
        nextList.removeAt(0);

        for(const Event e : nextList)
        {
            if(e.m_startDt.date() > endDate)
            {
                currentList.append(e);
                endDate = e.m_endDt.date();
                nextList.removeOne(e);
            }
        }
        for(DayInMonthItem* d : m_daysInMonthItems)
            d->setAppointmentRangeSlot(slotNum, currentList, m_weekStartDay);
        currentList.clear();
        slotNum++;
    }

    // all-day and same-day items
    for(DayInMonthItem* d : m_daysInMonthItems)
    {
        d->setAppointments(allDayList);
        d->setAppointments(dayList);
    }
}


void CalendarScene::setAppointmentsFor3Weeks(const QVector<Event> & list)
{
    for(DayInMonthItem* d : m_daysIn3WeeksItems)
        d->clearAppointments();
    if(list.isEmpty())
        return;
    QVector<Event> currentList;
    QVector<Event> nextList;
    QVector<Event> dayList;
    QVector<Event> allDayList;

    // dispatch all appointments to day and a range lists
    for(Event e: list)
    {
        /* @fixme:
         * if(apm->sameDay())
        {
            if(apm->m_appointmentData.m_allDay)
                allDayList.append(apm);
            else
                dayList.append(apm);
        }
        else*/
            nextList.append(e);
    }

    // range items
    int slotNum = 0;
    QDate endDate;
    while( ! nextList.isEmpty())
    {
        currentList.append(nextList[0]);
        endDate = nextList[0].m_endDt.date();
        nextList.removeAt(0);

        for(const Event e : nextList)
        {
            if(e.m_startDt.date() > endDate)
            {
                currentList.append(e);
                endDate = e.m_endDt.date();
                nextList.removeOne(e);
            }
        }
        for(DayInMonthItem* d : m_daysIn3WeeksItems)
            d->setAppointmentRangeSlot(slotNum, currentList, m_weekStartDay);
        currentList.clear();
        slotNum++;
    }

    // all-day and same-day items
    for(DayInMonthItem* d : m_daysIn3WeeksItems)
    {
        d->setAppointments(allDayList);
        d->setAppointments(dayList);
    }
}


void CalendarScene::setAppointmentsForWeek(const QVector<Event> & list)
{
    for(DayInWeekItem* d : m_daysInWeekItems)
        d->clearAppointments();
    if(list.isEmpty())
        return;
    QVector<Event> currentList;
    QVector<Event> nextList;
    QVector<Event> dayList;
    QVector<Event> allDayList;

    // dispatch all appointments to day and a range lists
    for(Event e : list)
    {
        /* @fixme: if(apm->sameDay())
        {
            if(apm->m_appointmentData.m_allDay)
                allDayList.append(apm);
            else
                dayList.append(apm);
        }
        else*/
            nextList.append(e);
    }

    // range items
    int slotNum = 0;
    QDate endDate;
    while( ! nextList.isEmpty())
    {
        currentList.append(nextList[0]);
        endDate = nextList[0].m_endDt.date();
        nextList.removeAt(0);

        for(const Event e : nextList)
        {
            if(e.m_startDt.date() > endDate)
            {
                currentList.append(e);
                endDate = e.m_endDt.date();
                nextList.removeOne(e);
            }
        }
        for(DayInWeekItem* d : m_daysInWeekItems)
            d->setAppointmentRangeSlot(slotNum, currentList, m_weekStartDay);
        currentList.clear();
        slotNum++;
    }

    // all-day and same-day items
    for(DayInWeekItem* d : m_daysInWeekItems)
    {
        d->setAppointments(allDayList);
        d->setAppointments(dayList);
    }
}


void CalendarScene::setAppointmentsForDay(const QVector<Event> &list)
{
    m_dayInDayItem->clearAppointments();
    if(list.isEmpty()) return;

    QVector<Event> fullDayList;
    QVector<Event> partDayList;

    // dispatch all appointments to fullDay and part-time-list.
    for(Event e : list)
    {
        if(e.m_startDt.date() < m_currentBaseDate and e.m_endDt.date() > m_currentBaseDate)
            fullDayList.append(e);
        /* @fixme: else if(apm->m_appointmentData.m_allDay)
            fullDayList.append(apm);*/
        else
            partDayList.append(e);
    }
    // send them to item
    m_dayInDayItem->setAppointmentsFullDay(fullDayList);
    m_dayInDayItem->setAppointmentsPartDay(partDayList);
}


void CalendarScene::removeAllEvents()
{
    for( DayInYearItem* itm : m_daysInYearItems )
        itm->clearAppointments();
    for( DayInMonthItem* itm : m_daysInMonthItems )
        itm->clearAppointments();
    for( DayInMonthItem* itm : m_daysIn3WeeksItems )
        itm->clearAppointments();
    for( DayInWeekItem* itm : m_daysInWeekItems )
        itm->clearAppointments();
    m_dayInDayItem->clearAppointments();
}


void CalendarScene::setSettings(const SettingsData & settings)
{
    m_weekStartDay = settings.m_weekStartDay;   // local copy

    // header for week
    int day = settings.m_weekStartDay;
    for(HeaderWeekday* hw : m_headersWeekday)
    {
        hw->setWeekday(day);
        day++;
        if(day > 7) day = 1;
    }

    m_dayInDayItem->setActiveDaytime(settings.m_dayStartHour, settings.m_dayEndHour);

    // update
    setDate(m_currentBaseDate, true);
}


void CalendarScene::eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor )
{
    for( DayInYearItem* itm : m_daysInYearItems )
        itm->eventsHaveNewColor( inUsercalendarID, inCalendarColor );
    for( DayInMonthItem* itm : m_daysInMonthItems )
        itm->eventsHaveNewColor( inUsercalendarID, inCalendarColor );
    for( DayInMonthItem* itm : m_daysIn3WeeksItems )
        itm->eventsHaveNewColor( inUsercalendarID, inCalendarColor );
    for( DayInWeekItem* itm : m_daysInWeekItems )
        itm->eventsHaveNewColor( inUsercalendarID, inCalendarColor );
    m_dayInDayItem->eventsHaveNewColor( inUsercalendarID, inCalendarColor );

}


void CalendarScene::createHeader()
{
    for(int i = 1; i <= 12; i++)
    {
        HeaderMonth* tmp = new HeaderMonth(i);
        tmp->setPos((i-1) * tmp->boundingRect().width(), 0 );
        tmp->hide();
        addItem(tmp);
        m_headersMonth.append(tmp);
    }

    for(int i = 1; i <= 7; i++)
    {
        HeaderWeekday* tmp = new HeaderWeekday(i);
        tmp->setPos((i-1) * tmp->boundingRect().width(), 0 );
        tmp->hide();
        addItem(tmp);
        m_headersWeekday.append(tmp);
    }
}


void CalendarScene::createDays()
{
    int daysInMonth[] = {31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    int x0 = 0;
    int y0 = 20;

    // Year-view
    for(int i = 0; i < 12; i++)
    {
        x0 = i * 100;

        for(int k = 0; k < daysInMonth[i]; k++)
        {
            QDate d = QDate::currentDate();
            d.setDate(d.year(), i + 1, k + 1);
            DayInYearItem *tmp = new DayInYearItem(d);
            connect(tmp, SIGNAL(signalReconfigureAppointment(QString)), this, SIGNAL(signalReconfigureAppointment(QString)));
            connect(tmp, SIGNAL(signalDeleteAppointment(QString)), this, SIGNAL(signalDeleteAppointment(QString)));
            connect(tmp, SIGNAL(signalDateClicked(QDate)), this, SIGNAL(signalDateClicked(QDate)));
            tmp->setPos(x0, (k + 1) * 20);
            tmp->hide();
            addItem(tmp);
            m_daysInYearItems.append(tmp);
        }
    }

    // month (6 weeks) view
    QDate d = QDate::currentDate();
    d.setDate(d.year(), d.month(), 1);
    d = d.addDays( 1 - d.dayOfWeek() );

    for(int week = 0; week < 6; week++)
    {
        x0 = 0;
        for(int day = 0; day < 7; day++)
        {
            DayInMonthItem *tmp = new DayInMonthItem(d);
            connect(tmp, SIGNAL(signalReconfigureAppointment(QString)), this, SIGNAL(signalReconfigureAppointment(QString)));
            connect(tmp, SIGNAL(signalDeleteAppointment(QString)), this, SIGNAL(signalDeleteAppointment(QString)));
            connect(tmp, SIGNAL(signalDateClicked(QDate)), this, SIGNAL(signalDateClicked(QDate)));
            tmp->setPos(x0, y0);
            x0 += 100;
            tmp->hide();
            addItem(tmp);
            m_daysInMonthItems.append(tmp);
            d = d.addDays(1);
        }
        y0 += 100;
    }

    // 3 weeks (each day is of type DayInMonth)
    d = QDate::currentDate();
    d = d.addDays( 1 - d.dayOfWeek() );
    d = d.addDays(-7);
    for(int day = 0; day < 21; day++)
    {
        DayInMonthItem *tmp = new DayInMonthItem(d);
        connect(tmp, SIGNAL(signalReconfigureAppointment(QString)), this, SIGNAL(signalReconfigureAppointment(QString)));
        connect(tmp, SIGNAL(signalDeleteAppointment(QString)), this, SIGNAL(signalDeleteAppointment(QString)));
        connect(tmp, SIGNAL(signalDateClicked(QDate)), this, SIGNAL(signalDateClicked(QDate)));
        tmp->hide();
        addItem(tmp);
        m_daysIn3WeeksItems.append(tmp);
        d = d.addDays(1);
    }

    // week (7 days) view
    d = QDate::currentDate();
    d = d.addDays( 1 - d.dayOfWeek() );
    for(int day = 0; day < 7; day++)
    {
        DayInWeekItem *tmp = new DayInWeekItem(d);
        connect(tmp, SIGNAL(signalReconfigureAppointment(QString)), this, SIGNAL(signalReconfigureAppointment(QString)));
        connect(tmp, SIGNAL(signalDeleteAppointment(QString)), this, SIGNAL(signalDeleteAppointment(QString)));
        connect(tmp, SIGNAL(signalDateClicked(QDate)), this, SIGNAL(signalDateClicked(QDate)));
        tmp->hide();
        addItem(tmp);
        m_daysInWeekItems.append(tmp);
        d = d.addDays(1);
    }

    // single day
    m_dayInDayItem = new DayInDayItem(QDate::currentDate());
    connect(m_dayInDayItem, SIGNAL(signalReconfigureAppointment(QString)), this, SIGNAL(signalReconfigureAppointment(QString)));
    connect(m_dayInDayItem, SIGNAL(signalDeleteAppointment(QString)), this, SIGNAL(signalDeleteAppointment(QString)));
    connect(m_dayInDayItem, SIGNAL(signalDateClicked(QDate)), this, SIGNAL(signalDateClicked(QDate)));
    m_dayInDayItem->hide();
    addItem(m_dayInDayItem);

}


void CalendarScene::hideAllItems()
{
    // hide all headers
    for(HeaderWeekday *h : m_headersWeekday)
        h->hide();
    for(HeaderMonth *h : m_headersMonth)
        h->hide();
    // hide days
    for(DayInYearItem *d : m_daysInYearItems)
        d->hide();
    for(DayInMonthItem *d : m_daysInMonthItems)
        d->hide();
    for(DayInMonthItem *d : m_daysIn3WeeksItems)
        d->hide();
    for(DayInWeekItem *d : m_daysInWeekItems)
        d->hide();
    m_dayInDayItem->hide();
}


void CalendarScene::slotShowYear()
{
    hideAllItems();
    for(HeaderMonth *h : m_headersMonth)
    {
        h->show();
    }
    for(DayInYearItem *d : m_daysInYearItems)
    {
        if(d->date().isValid())
            d->show();
    }
    m_showView = CalendarShow::SHOW_YEAR;
}


void CalendarScene::slotShowMonth()
{
    hideAllItems();
    for(HeaderWeekday *h : m_headersWeekday)
    {
        h->show();
    }
    for(DayInMonthItem *d : m_daysInMonthItems)
    {
        d->show();
    }
    m_showView = CalendarShow::SHOW_MONTH;
}


void CalendarScene::slotShow3Weeks()
{
    hideAllItems();
    for(HeaderWeekday *h : m_headersWeekday)
    {
        h->show();
    }
    for(DayInMonthItem *d : m_daysIn3WeeksItems)
    {
        d->show();
    }
    m_showView = CalendarShow::SHOW_3WEEKS;
}


void CalendarScene::slotShowWeek()
{
    hideAllItems();
    for(HeaderWeekday *h : m_headersWeekday)
    {
        h->show();
    }
    for(DayInWeekItem *d : m_daysInWeekItems)
    {
        d->show();
    }
    m_showView = CalendarShow::SHOW_WEEK;
}


void CalendarScene::slotShowDay()
{
    hideAllItems();
    m_dayInDayItem->show();
    m_showView = CalendarShow::SHOW_DAY;
}



