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
#ifndef NAVIGATIONDIALOG_H
#define NAVIGATIONDIALOG_H

#include <QDialog>
#include <QCalendarWidget>



/* This is just a standard dialog without buttons and borders. The dialog has a fixed location
 * and is shown/hidden by action in MainWindow.
 * Signals:
 *  1. this dialog navigates between dates and the predefined QCalendarWidget is fine for that
 *  2. clicking on a date makes this day the current day
 *  3. double clicking on a date creates an appointment for this date.
*/
class NavigationDialog : public QDialog
{
    Q_OBJECT
public:
    explicit NavigationDialog(QWidget* parent = 0);
    ~NavigationDialog();

private:
    QCalendarWidget* m_calendarWidget;

signals:
    void signalActivated(const QDate & date);
    void signalClicked(const QDate & date);
    void signalCurrentPageChanged(int year, int month);

public slots:
    void slotSetSelectedDate(const QDate & date);
};

#endif // NAVIGATIONDIALOG_H
