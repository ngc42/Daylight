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
#ifndef CALENDARMANAGERDIALOG_H
#define CALENDARMANAGERDIALOG_H

#include <QDialog>
#include <QListWidgetItem>

#include "usercalendar.h"


namespace Ui {
    class CalendarManagerDialog;
}


/* CalendarManagerDialog is a dialog, where the user can delete or modify a calendar.
 * UI-part of this dialog can be found in calendarmanagerdialog.ui.
 * This dialog is controlled in MainWindow, as a result of menu action.
 */
class CalendarManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CalendarManagerDialog(const QList<UserCalendarInfo*> calList, QWidget* parent);
    ~CalendarManagerDialog();

private:
    Ui::CalendarManagerDialog* m_ui;
    QList<UserCalendarInfo*> m_calListCopy;

    void showUserCalendars();

private slots:
    void slotModifyCalendar();
    void slotDeleteCalendar();
    void slotCurrentItemChanged(QListWidgetItem* currentItem, QListWidgetItem* previousItem);


signals:
    void signalDeleteCalendar(const int calId);
    void signalModifyCalendar(const int calId, const QString & title, const QColor & color);
};

#endif // CALENDARMANAGERDIALOG_H
