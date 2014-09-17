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
#ifndef CALENDARHEADER_H
#define CALENDARHEADER_H

#include <QPainter>
#include <QGraphicsTextItem>
#include <QDate>



/* Just the base class to display Month (display for a full year) or Week (Monday, ..., Sunday)
 * on top of a view.
 * The header are configured in CalendarScene (calendarscene.h). */
class CalendarHeader : public QGraphicsItem
{
public:
    CalendarHeader(QGraphicsItem* parent = 0);
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    void setTitle(const QString & title);
    void resize(const qreal width, const qreal height);
    int perfectFontSizeForString(const QString & text, const int width, const int height) const;
    void setFontPixelSize(int size);

protected:
    QGraphicsTextItem* m_textItem;
    void setTextItemPos();
    QSizeF m_size;
};



/* Shows the month names, in a long (January) or a short (Jan) way. */
class HeaderMonth : public CalendarHeader
{
public:
    HeaderMonth(const int month, QGraphicsItem* parent = 0);
    QRectF boundingRect() const { return QRectF({0, 0}, m_size); }
    void setShowLong(const bool showLongName);

private:
    QString m_longMonthName;
    QString m_shortMonthName;
};



/* Shows the day names, in a long (Monday) or a short (Mon) way. */
class HeaderWeekday : public CalendarHeader
{
public:
    HeaderWeekday(const int weekday, QGraphicsItem* parent = 0);
    void setWeekday(const int weekday);         // weekday may change, depends on settings
    QRectF boundingRect() const { return QRectF({0, 0}, m_size); }
    void setShowLong(const bool showLongName);

private:
    QString m_longDayName;
    QString m_shortDayName;
    bool m_showLongName;
};


#endif // CALENDARHEADER_H
