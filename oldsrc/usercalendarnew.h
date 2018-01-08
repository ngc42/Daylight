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
#ifndef USERCALENDARNEW_H
#define USERCALENDARNEW_H

#include <QDialog>
#include <QColor>
#include <usercalendar.h>



namespace Ui {
class UserCalendarNew;
}



/* UserCalendarNew is a dialog to create a new user calendar.
 * It contains a list of currently available user calendars and
 *  some elements to create a new calendar.
 * UI is in usercalendarnew.ui. */
class UserCalendarNew : public QDialog
{
    Q_OBJECT

public:
    explicit UserCalendarNew(QWidget* parent = 0);
    ~UserCalendarNew();
    QString calendarTitle() const;
    QColor calendarColor() const;
    void reset();
    void setUserCalendarInfos(const QList<UserCalendarInfo*> uciList);

private:
    void setupColorCombo();

    Ui::UserCalendarNew *m_ui;
};

#endif // USERCALENDARNEW_H
