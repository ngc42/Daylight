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
#include "dayitem.h"
#include <QDebug>

/***********************************************************
********** TooManyEventsItem *************************
***********************************************************/

TooManyEventsItem::TooManyEventsItem(QGraphicsItem* parent) :
    QGraphicsItem(parent), m_size(5, 5)
{
    setToolTip("Too many events. I am sorry but we cannot show them all.");
    hide();
}


void TooManyEventsItem::resize(const qreal width, const qreal height)
{
    m_size = QSizeF((int) width, (int) height);
    update();
}


void TooManyEventsItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* , QWidget*)
{
    painter->fillRect(boundingRect(), Qt::lightGray);
    painter->setPen(Qt::black);
    painter->drawText(boundingRect(), "…", Qt::AlignVCenter | Qt::AlignHCenter);
}



/***********************************************************
********** EventItem ***************************************
***********************************************************/

EventItem::EventItem(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_color(Qt::black), m_dummy(true), m_size(3, 3), m_sizeTooSmall(true),
    m_title(""), m_showTitle(false), m_fontPixelSize(0), m_appointmentId("")
{
    //dummy items should not paint anything!
    setFlag(QGraphicsItem::ItemHasNoContents, true);
}


EventItem::EventItem(Event event, QGraphicsItem *parent) :
    QGraphicsObject(parent),
    m_color(event.m_eventColor),m_userCalendarId(event.m_userCalendarId),
    m_dummy(false), m_size(3, 3), m_sizeTooSmall(false),
    m_title(event.m_displayText),
    m_showTitle(false), m_fontPixelSize(1),

    m_appointmentId(event.m_uid),
    m_startDt(event.m_startDt),
    m_endDt(event.m_endDt), m_allDay(false)
{
    QString toolTipText = QString("%1 (cal-id = %2, app-id = %3) - %4 to %5")
            .arg(m_title)
            .arg(m_userCalendarId).arg(m_appointmentId)
            .arg(m_startDt.toString("dd.MM.yy, hh:mm")).arg(m_endDt.toString("dd.MM.yy, hh:mm"));
    setToolTip(toolTipText);
    setFlag(QGraphicsItem::ItemIsFocusable, true);

    // context menu
    m_actionReconfigureAppointment = new QAction("Configure", this);
    connect(m_actionReconfigureAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareReconfigureAppointment()));
    m_deleteThisAppointment = new QAction("Delete Appointment", this);
    connect(m_deleteThisAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareDeleteAppointment()));
    m_contextMenu.addAction(m_actionReconfigureAppointment);
    m_contextMenu.addAction(m_deleteThisAppointment);
}


EventItem::~EventItem()
{
    disconnect(m_actionReconfigureAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareReconfigureAppointment()));
    disconnect(m_deleteThisAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareDeleteAppointment()));
    delete m_actionReconfigureAppointment;
    delete m_deleteThisAppointment;
}


void EventItem::resize(const qreal width, const qreal height)
{
    if(m_dummy) return;
    m_size = QSizeF((int) width, (int) height);
    m_sizeTooSmall = width < 1.0f or height <1.0f;
    update();
}


void EventItem::setFontPixelSize(int size)
{
    m_fontPixelSize = size > 0 ? size : 5;
}


void EventItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if(!m_sizeTooSmall)
    {
        painter->fillRect(boundingRect(), m_color);
        if(m_showTitle)
        {
            bool onlyThisDay = m_startDt.date() == m_endDt.date();
            QString s = QString("%1%2").arg(onlyThisDay?"•":"").arg(m_title);
            QSizeF m(-1, -1);
            QRectF r = QRectF({0, 0}, boundingRect().size() + m);
            painter->drawText(r, s, Qt::AlignVCenter | Qt::AlignLeft);
        }

    }
}


void EventItem::setShowTitle(const bool show)
{
    if(m_dummy) return;
    m_showTitle = show;
    update();
}


void EventItem::keyReleaseEvent(QKeyEvent* event)
{
    switch(event->key())
    {
        case Qt::Key_Return:
            slotPrepareReconfigureAppointment();
            break;
        case Qt::Key_Delete:
            slotPrepareDeleteAppointment();
            break;
        default:
            break;
    }
}


void EventItem::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    if(event->button() == Qt::LeftButton)
        slotPrepareReconfigureAppointment();
    QGraphicsObject::mousePressEvent(event);
}


void EventItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
    m_contextMenu.exec(event->screenPos());
}


void EventItem::slotPrepareReconfigureAppointment()
{
    if(m_appointmentId == "" or m_dummy)
        return;
    emit signalReconfigureAppointment(m_appointmentId);
}


void EventItem::slotPrepareDeleteAppointment()
{
    if(m_appointmentId == "" or m_dummy)
        return;
    disconnect(m_actionReconfigureAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareReconfigureAppointment()));
    disconnect(m_deleteThisAppointment, SIGNAL(triggered()), this, SLOT(slotPrepareDeleteAppointment()));
    // this is fragile, as this causes the deletion of "this" during execution. Qt::QueuedConnection is fine.
    QMetaObject::invokeMethod(this, "signalDeleteAppointment", Qt::QueuedConnection, Q_ARG(QString, m_appointmentId) );
}



/***********************************************************
********** DayItem *****************************************
***********************************************************/

DayItem::DayItem(QGraphicsItem *parent) :
    QGraphicsObject(parent), m_displayDate(QDate::currentDate()), m_size(100, 20)
{
    m_dayLabel = new QGraphicsSimpleTextItem(this);
}


DayItem::~DayItem()
{
    delete m_dayLabel;
}


void DayItem::resize(const qreal width, const qreal height)
{
    m_size = QSizeF(width, height);
}


int DayItem::perfectFontSizeForString(const QString & text, const int width, const int height) const
{
    QFont f = m_dayLabel->font();
    f.setWeight(QFont::Normal);
    int s = height;
    while(s > 1)
    {
        f.setPixelSize(s);
        QFontMetrics metrics1 = QFontMetrics(f);
        if(metrics1.width(text) <= width)
            return s;
        s--;
    }
    return 1;
}


void DayItem::setFontPixelSize(const int size)
{
    QFont f = m_dayLabel->font();
    f.setPixelSize(size <= 0 ? 1 : size);
    m_dayLabel->setFont(f);
}


void DayItem::setDate(const QDate date)
{
    m_displayDate = date;
    if(! date.isValid())
    {
        return;
    }
    QDate cDate = QDate::currentDate();
    QFont font = m_dayLabel->font();
    bool weekend = date.dayOfWeek() > 5;

    if(cDate.year() == date.year() and cDate.month() == date.month())
    {
        if(cDate.day() == date.day())
        {
            font.setWeight(QFont::Bold);
            m_dayLabel->setFont(font);
            m_dayLabel->setBrush(weekend ? Qt::red : Qt::blue);
        }
        else
        {
            font.setWeight(QFont::Normal);
            m_dayLabel->setFont(font);
            m_dayLabel->setBrush(weekend ? Qt::red : Qt::darkBlue);
        }
    }
    else
    {
        font.setWeight(QFont::Normal);
        m_dayLabel->setFont(font);
        m_dayLabel->setBrush(weekend ? Qt::darkRed : Qt::black);
    }
}


void DayItem::setDayText(const QString dayString)
{
    m_dayLabel->setText(dayString);
}


void DayItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent*)
{
    emit signalDateClicked(m_displayDate);
}


void DayItem::slotDeleteAppointment(QString appointmentId)
{
    emit signalDeleteAppointment(appointmentId);
}



/***********************************************************
********** DayInYearItem ***********************************
***********************************************************/

DayInYearItem::DayInYearItem(const QDate date, QGraphicsItem *parent) :
    DayItem(parent)
{
    m_weekNumberLabel = new QGraphicsSimpleTextItem("00", this);
    m_weekNumberLabel->setBrush(Qt::lightGray);
    m_tooManyItems = new TooManyEventsItem(this);
    m_tooManyItems->show();
    setDate(date);
    adjustSubitemPositions();
}


DayInYearItem::~DayInYearItem()
{
    clearAppointments();
    delete m_weekNumberLabel;
    delete m_tooManyItems;
}


void DayInYearItem::resize(const qreal width, const qreal height)
{
    DayItem::resize(width, height);
    adjustSubitemPositions();
}


void DayInYearItem::setFontPixelSize(const int size)
{
    DayItem::setFontPixelSize(size <= 0 ? 1 : size);
    QFont f = m_weekNumberLabel->font();
    f.setPixelSize(size <= 0 ? 1 : size);
    m_weekNumberLabel->setFont(f);
}


void DayInYearItem::adjustSubitemPositions()
{
    m_tooManyItems->hide();

    QSizeF mySize = boundingRect().size();
    qreal height = mySize.height();
    qreal width = mySize.width();

    // Appointment items
    qreal apiWidth = 2.0f;
    qreal x = width / 3.0f;
    qreal apiHeight = height - 2.0f;

    for(EventItem* itm : m_appointmentSlotsRange)
    {
        itm->resize(apiWidth, apiHeight);
        itm->setPos(x, 1.0f);
        if(x + apiWidth > width)
        {
            m_tooManyItems->show();
            itm->hide();
        }
        else
            itm->show();    // just in case it was previously hidden
        x = x + 2.0f * apiWidth;
    }

    for(EventItem* itm : m_appointmentSlotsDay)
    {
        itm->resize(apiWidth, apiHeight);
        itm->setPos(x, 1.0f);
        if(x + apiWidth > width)
        {
            m_tooManyItems->show();
            itm->hide();
        }
        else
            itm->show();    // just in case it was previously hidden
        x = x + 2.0f * apiWidth;
    }

    m_dayLabel->setPos(0, 0);
    m_weekNumberLabel->setPos(0, height / 2.0f);
    m_tooManyItems->resize(width / 6.0f, height/ 2.0f - 1.0f);
    m_tooManyItems->setPos(width / 6.0f, height / 2.0f);
}


void DayInYearItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::lightGray);
    painter->drawLine(boundingRect().topLeft(), boundingRect().topRight());
    painter->drawLine(boundingRect().topRight(), boundingRect().bottomRight());

    // the following very ugly code fixes display problems.
    // - sometimes, drawn lines of neighbours do not overlap. This is maybe because we use
    //   floating point arithmetics to calculate size and position of items.
    // - maybe, I should use dummy items or think about integer arithmetic.
    // To see the problem: comment this out and resize in "Show Year"-mode.
    // This is a follow up bug on use drawLine() instead of drawRect(boundingRect())

    QDate day = date();
    if(day.month() == 1)
    {
        painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
    }
    else if(day.month() == 3)
    {
        int d = day.isLeapYear(day.year()) ? 29 : 28;
        if(day.day() > d)
            painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
    }
    else if(day.day() == 31 and (day.month() == 5 or day.month() == 7 or day.month() == 10 or day.month() == 12))
    {
        painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
    }

    if(day.day() == day.daysInMonth())
    {
        painter->drawLine(boundingRect().bottomLeft(), boundingRect().bottomRight());
    }
}


void DayInYearItem::setDate(const QDate date)
{
    QString displayDay = date.toString("dd ddd");
    setDayText(displayDay);
    DayItem::setDate(date);
    if(date.day() == 1 or date.dayOfWeek() == 1)
    {
        QString displayWeekNumber = QString("%1").arg(date.weekNumber(), 2);
        m_weekNumberLabel->setText(displayWeekNumber);
        m_weekNumberLabel->show();
        adjustSubitemPositions();
    }
    else
        m_weekNumberLabel->hide();
}


void DayInYearItem::setAppointmentDaySlots(const QVector<Event> &list)
{
    if( ! date().isValid() ) return;
    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            EventItem* itm = new EventItem(e, this);
            connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            itm->setShowTitle(false);
            m_appointmentSlotsDay.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInYearItem::setAppointmentRangeSlot(const int slot, const QVector<Event> &list)
{
    if( ! date().isValid() ) return;
    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            while(slot > m_appointmentSlotsRange.count())
            {
                EventItem* itm = new EventItem(this);
                m_appointmentSlotsRange.append(itm);
            }
            EventItem* itm = new EventItem(e, this);
            //connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            //connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            itm->setShowTitle(false);
            m_appointmentSlotsRange.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInYearItem::clearAppointments()
{
    for(EventItem* itm : m_appointmentSlotsDay)
        delete itm;
    for(EventItem* itm : m_appointmentSlotsRange)
        delete itm;
    m_appointmentSlotsDay.clear();
    m_appointmentSlotsRange.clear();
    m_tooManyItems->hide();
}


void DayInYearItem::eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor)
{
    for(EventItem* itm : m_appointmentSlotsDay)
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
    for(EventItem* itm : m_appointmentSlotsRange)
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
}


/***********************************************************
********** DayInMonthItem **********************************
***********************************************************/

DayInMonthItem::DayInMonthItem(const QDate date, QGraphicsItem* parent) :
    DayItem(parent)
{
    m_tooManyItems = new TooManyEventsItem(this);
    setDate(date);
    adjustSubitemPositions();
}


DayInMonthItem::~DayInMonthItem()
{
    clearAppointments();
    delete m_tooManyItems;
}


void DayInMonthItem::resize(const qreal width, const qreal height)
{
    DayItem::resize(width, height);
    adjustSubitemPositions();
}


void DayInMonthItem::adjustSubitemPositions()
{
    m_tooManyItems->hide();

    QSizeF mySize = boundingRect().size();
    qreal height = mySize.height();

    qreal apiWidth = mySize.width() - 2.0f;
    qreal apiHeight= 15.0f;
    qreal y = apiHeight;

    for(EventItem* itm : m_appointmentSlots)
    {
        itm->resize(apiWidth, apiHeight);
        itm->setFontPixelSize(10);
        itm->setPos(1.0f, y);
        if(y + apiHeight > height)
        {
            itm->hide();
            m_tooManyItems->show();
        }
        else
            itm->show();    // in case it was hidden the last time
        y = y + apiHeight + 1.0f;
    }

    // to many items - position
    QPointF dayLabelPos = m_dayLabel->pos();
    QSizeF dayLabelSize = m_dayLabel->boundingRect().size();
    m_tooManyItems->resize(dayLabelSize.width(), dayLabelSize.height());
    m_tooManyItems->setPos(dayLabelPos.x() + dayLabelSize.width() + 1, dayLabelPos.y());
}


void DayInMonthItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::lightGray);
    painter->drawLine(boundingRect().topLeft(), boundingRect().topRight());
    painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
}


void DayInMonthItem::setDate(const QDate date)
{
    setDayText(date.toString("dd"));
    DayItem::setDate(date);
}


void DayInMonthItem::setAppointmentRangeSlot(const int slot, const QVector<Event> &list, int weekStart)
{
    if( (! date().isValid()) or list.isEmpty() ) return;

    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            while(slot > m_appointmentSlots.count())
            {
                EventItem* dummyItem = new EventItem(this);
                m_appointmentSlots.append(dummyItem);
            }

            EventItem* itm = new EventItem(e, this);
            connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            bool showTitle = (date().dayOfWeek() == weekStart) or
                    (e.m_startDt.date().day() == date().day() and
                     e.m_startDt.date().month() == date().month());
            itm->setShowTitle(showTitle);
            m_appointmentSlots.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInMonthItem::setAppointments(const QVector<Event> &list)
{
    if( (! date().isValid()) or list.isEmpty() ) return;
    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            bool replaced = false;

            EventItem* itm = new EventItem(e, this);
            connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            itm->setShowTitle(true);

            //find and replace dummy-item
            for(int index = 0; index < m_appointmentSlots.count(); index++)
            {
                if(m_appointmentSlots[index]->dummy())
                {
                    m_appointmentSlots.replace(index, itm);
                    replaced = true;
                    break;
                }
            }
            // else append item
            if(! replaced)
                m_appointmentSlots.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInMonthItem::clearAppointments()
{
    for(EventItem* itm : m_appointmentSlots)
        delete itm;
    m_appointmentSlots.clear();
}


void DayInMonthItem::eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor)
{
    for(EventItem* itm : m_appointmentSlots)
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
}


/***********************************************************
********** DayInWeekItem ***********************************
***********************************************************/

DayInWeekItem::DayInWeekItem(const QDate date, QGraphicsItem* parent) :
    DayItem(parent)
{
    m_tooManyItems = new TooManyEventsItem(this);
    setDate(date);
    adjustSubitemPositions();
}


DayInWeekItem::~DayInWeekItem()
{
    clearAppointments();
    delete m_tooManyItems;
}


void DayInWeekItem::resize(const qreal width, const qreal height)
{
    DayItem::resize(width, height);
    adjustSubitemPositions();
}


void DayInWeekItem::adjustSubitemPositions()
{
    m_tooManyItems->hide();

    QSizeF mySize = boundingRect().size();
    qreal height = mySize.height();

    qreal apiWidth = mySize.width() - 2.0f;
    qreal apiHeight= 15.0f;
    qreal y = apiHeight;

    for(EventItem* itm : m_appointmentSlots)
    {
        itm->resize(apiWidth, apiHeight);
        itm->setFontPixelSize(10);
        itm->setPos(1.0f, y);
        if(y + apiHeight > height)
        {
            itm->hide();
            m_tooManyItems->show();
        }
        else
            itm->show();    // in case it was hidden the last time
        y = y + apiHeight + 1.0f;
    }

    // to many items - position
    QPointF dayLabelPos = m_dayLabel->pos();
    QSizeF dayLabelSize = m_dayLabel->boundingRect().size();
    m_tooManyItems->resize(dayLabelSize.width(), dayLabelSize.height());
    m_tooManyItems->setPos(dayLabelPos.x() + dayLabelSize.width() + 1, dayLabelPos.y());
}


void DayInWeekItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen(Qt::lightGray);
    painter->drawLine(boundingRect().topLeft(), boundingRect().topRight());
    painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
}


void DayInWeekItem::setDate(const QDate date)
{
    setDayText(date.toString("dd"));
    DayItem::setDate(date);
}


void DayInWeekItem::setAppointmentRangeSlot(const int slot, const QVector<Event> &list, int weekStart)
{
    if( (! date().isValid()) or list.isEmpty() ) return;

    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            while(slot > m_appointmentSlots.count())
            {
                EventItem* dummyItem = new EventItem(this);
                m_appointmentSlots.append(dummyItem);
            }

            EventItem* itm = new EventItem(e, this);
            connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            bool showTitle = (date().dayOfWeek() == weekStart) or
                    (e.m_startDt.date().day() == date().day() and
                     e.m_startDt.date().month() == date().month());
            itm->setShowTitle(showTitle);
            m_appointmentSlots.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInWeekItem::setAppointments(const QVector<Event> &list)
{
    if( (! date().isValid()) or list.isEmpty() ) return;
    for(Event e : list)
    {
        if( e.containsDay(date()) )
        {
            bool replaced = false;

            EventItem* itm = new EventItem(e, this);
            connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
            connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
            itm->setShowTitle(true);

            //find and replace dummy-item
            for(int index = 0; index < m_appointmentSlots.count(); index++)
            {
                if(m_appointmentSlots[index]->dummy())
                {
                    m_appointmentSlots.replace(index, itm);
                    replaced = true;
                    break;
                }
            }
            // else append item
            if(! replaced)
                m_appointmentSlots.append(itm);
        }
    }
    adjustSubitemPositions();
}


void DayInWeekItem::clearAppointments()
{
    for(EventItem* itm : m_appointmentSlots)
        delete itm;
    m_appointmentSlots.clear();
}


void DayInWeekItem::eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor)
{
    for(EventItem* itm : m_appointmentSlots)
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
}


/***********************************************************
********** DayInDayItem ************************************
***********************************************************/

DayInDayItem::DayInDayItem(const QDate date, QGraphicsItem* parent) :
    DayItem(parent), m_activeHourStart(8), m_activeHourEnd(20), m_numberOfAppointmentOverlaps(0)
{
    m_tooManyItems = new TooManyEventsItem(this);
    setDate(date);
}


DayInDayItem::~DayInDayItem()
{
    clearAppointments();
    delete m_tooManyItems;
}


void DayInDayItem::resize(const qreal width, const qreal height)
{
    DayItem::resize(width, height);
    createMarkerList();
    adjustSubitemPositions();
}


void DayInDayItem::adjustSubitemPositions()
{
    m_tooManyItems->hide();

    QSizeF mySize = boundingRect().size();

    qreal apiWidth = mySize.width() - 2.0f;
    qreal apiHeight= 15.0f;
    qreal y = apiHeight;

    int apmfdHeight = m_markerList[0].ypos;     // height of the area, where the full-day items live

    for(EventItem* itm : m_appointmentFullDay)
    {
        itm->resize(apiWidth, apiHeight);
        itm->setFontPixelSize(10);
        itm->setPos(1.0f, y);
        if(y + apiHeight > apmfdHeight)
        {
            itm->hide();
            m_tooManyItems->show();
        }
        else
            itm->show();    // in case it was hidden the last time
        y = y + apiHeight + 1.0f;
    }

    apiWidth = apiWidth / (m_numberOfAppointmentOverlaps + 1) - 20 ;
    /* Idea behind bookkeeping:
     * Every deltatime (appointment) gets an own slot and registeres its timedelta to timeSlotBookkeeper by just +1 to the timeSlotBookkeeper's hour index.
     * Every new appointment checks, wether there is already a timedelta and reserveres the next slot,
     *  given by the maximum slots in the interval.
     * timeSlotBookkeeper[] with index 24 and 25 have special meaning: Time before day-interval (m_activeHourStart..m_activeHourEnd) and after. */
    int timeSlotBookkeeper[26];
    for(int i = 0; i < 26; i++) timeSlotBookkeeper[i] = 0;

    for(EventItem* itm : m_appointmentPartDay)
    {
        int startH = (itm->startDt().date() < date()) ? (m_activeHourStart > 0 ? m_activeHourStart - 1 : 0) : itm->startDt().time().hour();
        int endH = (itm->endDt().date() > date()) ? (m_activeHourEnd < 24 ? m_activeHourEnd + 1 : 24) : itm->endDt().time().hour();
        int hours =  endH - startH + 1;
        if(startH < m_activeHourStart) timeSlotBookkeeper[24]++;        // register ourselves to a slot.
        if(endH > m_activeHourEnd) timeSlotBookkeeper[25]++;
        for(int i = startH; i <= endH; i++) timeSlotBookkeeper[i]++;

        itm->resize(apiWidth, hours * m_deltaPixelForHour);
        itm->setFontPixelSize(10);

        // find out y-pos to set Appointment item
        int y = 0;
        if(startH < m_activeHourStart)
            y = m_markerList[0].ypos + 1;
        else
        {
            y = m_markerList[0].ypos + 1 + (startH - m_activeHourStart + 1) * m_deltaPixelForHour;
        }

        // x-position for appointment
        int x = 20;
        int max = 0;
        for(int i = 0; i < 24; i++)
        {
            if(i >= startH and i <= endH)
                max = max > timeSlotBookkeeper[i] ? max : timeSlotBookkeeper[i];    // find slot maximum, that is what we have registered
        }

        if(startH < m_activeHourStart)                                              // special: for range outside of day interval
            max = max > timeSlotBookkeeper[24] ? max : timeSlotBookkeeper[24];
        if(endH > m_activeHourEnd)
            max = max > timeSlotBookkeeper[25] ? max : timeSlotBookkeeper[25];
        x += (max - 1) * apiWidth;
        itm->setPos(x, y);
    }

    // to many items - position
    QPointF dayLabelPos = m_dayLabel->pos();
    QSizeF dayLabelSize = m_dayLabel->boundingRect().size();
    m_tooManyItems->resize(dayLabelSize.width(), dayLabelSize.height());
    m_tooManyItems->setPos(dayLabelPos.x() + dayLabelSize.width() + 1, dayLabelPos.y());
}


void DayInDayItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget* )
{
    painter->setPen(Qt::lightGray);
    QRectF itemRect = boundingRect();
    painter->drawLine(itemRect.topLeft(), itemRect.topRight());
    painter->drawLine(itemRect.topLeft(), itemRect.bottomLeft());

    for(Marker m : m_markerList)
    {
        painter->drawLine(QPointF(0.0f, m.ypos), QPointF(itemRect.right(), m.ypos));
        if(m.hour < 0 or m.hour == 25)
            continue;
        QString s = QString("%1").arg(m.hour, 2, 10, QLatin1Char('0'));
        painter->drawText(QPointF(0.0f, m.ypos), s);
    }
}


void DayInDayItem::setDate(const QDate date)
{
    setDayText(date.toString("dd"));
    DayItem::setDate(date);
}


void DayInDayItem::setActiveDaytime(int hourBegin, int hourEnd)
{
    m_activeHourStart = hourBegin;
    m_activeHourEnd = hourEnd;
    createMarkerList();
    update();
    adjustSubitemPositions();
}


void DayInDayItem::setAppointmentsFullDay(const QVector<Event> &list)
{
    if(list.isEmpty() or (!date().isValid())) return;
    for(Event e : list)
    {
        EventItem* itm = new EventItem(e, this);
        connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
        connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
        itm->setShowTitle(true);
        m_appointmentFullDay.append(itm);
    }
    adjustSubitemPositions();
}


void DayInDayItem::setAppointmentsPartDay(const QVector<Event> &list)
{
    m_numberOfAppointmentOverlaps = 0;    // calculate appointment overlaps to display correct width in DayInDayItem::adjustSubitemPositions()

    if(list.isEmpty() or (!date().isValid())) return;

    QVector<Event> sortedList = list;
    if(list.count() > 1)
    {
        int count = list.count();
        // start sorting by start time...
        bool swapped = true;
        while(swapped)
        {
            swapped = false;
            for(int i = 0; i < count - 1; i++)
            {
                if(sortedList[i].m_startDt.time() > sortedList[i+1].m_startDt.time())
                {
                    sortedList.move(i, i+1);
                    swapped = true;
                }
            }
        }
        //...finished sorting.

        // find overlaps of appointments...
        int overlapArray[26];
        for(int i = 0; i < 26; i++) overlapArray[i] = 0;
        for(int i = 0; i < count - 1; i++)
        {
            int iStartH = (sortedList[i].m_startDt.date() < date()) ? 0 : sortedList[i].m_startDt.time().hour();
            int iEndH = (sortedList[i].m_startDt.date() > date()) ? 24 : sortedList[i].m_startDt.time().hour();

            for(int h = iStartH; h <= iEndH; h++)
            {
                overlapArray[h]++;
            }
            if(iStartH < m_activeHourStart) overlapArray[24]++;   // previous time before visible day starts
            if(iEndH > m_activeHourEnd) overlapArray[25]++;
        }
        // maximum from overlapArray is now the overlap
        for(int i = 0; i < 26; i++)
        {
            m_numberOfAppointmentOverlaps = (overlapArray[i] > m_numberOfAppointmentOverlaps) ? overlapArray[i] : m_numberOfAppointmentOverlaps;
        }

        // ... finished
    }

    // create event items
    for(Event e : sortedList)
    {
        EventItem* itm = new EventItem(e, this);
        connect(itm, SIGNAL(signalReconfigureAppointment(int)), this, SIGNAL(signalReconfigureAppointment(int)));
        connect(itm, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)));
        itm->setShowTitle(true);
        m_appointmentPartDay.append(itm);
    }
    adjustSubitemPositions();
}


void DayInDayItem::clearAppointments()
{
    for(EventItem* itm : m_appointmentFullDay)
        delete itm;
    m_appointmentFullDay.clear();
    for(EventItem* itm : m_appointmentPartDay)
        delete itm;
    m_appointmentPartDay.clear();
}


void DayInDayItem::eventsHaveNewColor(const int inUsercalendarID, const QColor inCalendarColor)
{
    for( EventItem* itm : m_appointmentFullDay )
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
    for( EventItem* itm : m_appointmentPartDay )
    {
        if( itm->m_userCalendarId == inUsercalendarID )
        {
            itm->m_color = inCalendarColor;
            itm->update();
        }
    }
}


/* There are markers after a big section, where FullDay-Appointments live,
 * and between the hours. If start and end hour are away from midnight,
 * there is some space before start time and after end time.
 * These markers are created here and displayed during paint(). Furthermore,
 * they are used to calculate the place, where the AppointmentItems are placed. */
void DayInDayItem::createMarkerList()
{
    m_markerList.clear();
    QRectF itemRect = boundingRect();

    Marker top;                             // field for full day appointments
    top.ypos = itemRect.height() / 4;
    top.hour =  -2;

    int rest = itemRect.height() - top.ypos;
    int numItems = 1 + (m_activeHourEnd - m_activeHourStart);
    if(m_activeHourStart > 0) numItems++;   // time before active hours
    if(m_activeHourEnd < 24) numItems++;    // time after active hours

    m_deltaPixelForHour = rest / numItems;  // small vertical space to display an hour
    if(m_deltaPixelForHour > 0)
        top.ypos = top.ypos +  (rest % m_deltaPixelForHour);    // add possible left-over space to topspace
    m_markerList.append(top);               // append to list
    int height = top.ypos + m_deltaPixelForHour;

    if(m_activeHourStart > 0)               // Marker for section _before_ m_activeHourStart
    {
        Marker m;
        m.hour = -1;
        m.ypos = height;
        height += m_deltaPixelForHour;
        m_markerList.append(m);
    }
    for(int i = m_activeHourStart; i <= m_activeHourEnd; i++)
    {
        Marker m;
        m.hour = i;
        m.ypos = height;
        height += m_deltaPixelForHour;
        m_markerList.append(m);
    }
    if(m_activeHourEnd < 24)                // Marker for section _after_ m_activeHourEnd
    {
        Marker m;
        m.hour = 25;
        m.ypos = height;
        m_markerList.append(m);
    }
}

