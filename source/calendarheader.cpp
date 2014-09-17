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
#include <QFontMetrics>
#include "calendarheader.h"


/***********************************************************
********** CalendarHeader **********************************
***********************************************************/

CalendarHeader::CalendarHeader(QGraphicsItem* parent) :
    QGraphicsItem(parent), m_size(100, 20)
{
    m_textItem = new QGraphicsTextItem(this);
}


void CalendarHeader::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->fillRect(boundingRect(), QColor(0xBA, 0xD8, 0xff));
    painter->setPen(Qt::lightGray);
    painter->drawLine(boundingRect().topLeft(), boundingRect().topRight());
    painter->drawLine(boundingRect().topLeft(), boundingRect().bottomLeft());
}


void CalendarHeader::setTitle(const QString & title)
{
    m_textItem->setPlainText(title);
    setTextItemPos();
}


void CalendarHeader::resize(const qreal width, const qreal height)
{
    m_size = QSizeF(width, height);
    setTextItemPos();
}


int CalendarHeader::perfectFontSizeForString(const QString & text, const int width, const int height) const
{
    QFont f = m_textItem->font();
    f.setWeight(QFont::Normal);
    int s = height - 4;
    while(s > 5)
    {
        f.setPixelSize(s);
        QFontMetrics metrics1 = QFontMetrics(f);
        if(metrics1.width(text) < width)
            return s;
        s--;
    }
    return s;
}


void CalendarHeader::setFontPixelSize(int size)
{
    QFont f = m_textItem->font();
    f.setPixelSize(size);
    m_textItem->setFont(f);
}


void CalendarHeader::setTextItemPos()
{
    QRectF r = boundingRect();
    QRectF tir = m_textItem->boundingRect();
    qreal w = (r.width() - tir.width()) / 2.0f;
    qreal h = (r.height() - tir.height()) / 2.0f;
    m_textItem->setPos(w, h);
}



/***********************************************************
********** HeaderMonth *************************************
***********************************************************/

HeaderMonth::HeaderMonth(const int month, QGraphicsItem* parent) :
    CalendarHeader(parent)
{
    if(month >= 1 and month <= 12)
    {
        m_longMonthName = QDate::longMonthName(month);
        m_shortMonthName = QDate::shortMonthName(month);
    }
    else
    {
        m_longMonthName = "unknown";
        m_shortMonthName = "unknown";
    }
}


void HeaderMonth::setShowLong(const bool showLongName)
{
    setTitle(showLongName ? m_longMonthName : m_shortMonthName);
}



/***********************************************************
********** HeaderWeekday ***********************************
***********************************************************/

HeaderWeekday::HeaderWeekday(const int weekday, QGraphicsItem* parent) :
    CalendarHeader(parent), m_showLongName(true)
{
    setWeekday(weekday);
}


void HeaderWeekday::setWeekday(const int weekday)
{
    if(weekday >= 1 and weekday <= 7)
    {
        m_longDayName  = QDate::longDayName(weekday);
        m_shortDayName = QDate::shortDayName(weekday);
    }
    else
    {
        m_longDayName  = "unknown";
        m_shortDayName = "unknown";
    }
    setShowLong(m_showLongName);    // update
}


void HeaderWeekday::setShowLong(const bool showLongName)
{
    setTitle(showLongName ? m_longDayName : m_shortDayName);
    m_showLongName = showLongName;
}


