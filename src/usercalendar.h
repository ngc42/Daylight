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
#ifndef USERCALENDAR_H
#define USERCALENDAR_H

#include <QColor>
#include <QMenu>


/* representation of a user calendar. Except the action, this is what is stored in Storage. */
struct UserCalendarInfo
{
    int m_id;               // identifier of calendar
    QColor m_color;         // color of appointments belonging to this calendar
    QString m_title;        // name of this calendar, like "work", or "sport".
    bool m_isVisible;       // appointments inside this calendar currently visible?
    QAction *m_action;      // action for calendar menu
};



/* This UserCalendarPool contains a list with all user calendars, this program knows about.
 * This is populated at program start from storage, you may modify the pool by adding, modifying
 *  or removing a user calendar, mostly with CalendarManagerDialog.
 * A UC is read with addUserCalendarFromStorage() from storage.
 * If you add a calendar, a unique ID (type int, so not endless many ids!) is given.
 * Every user calendar has a menu representation, so you can switch it on/off via menu actions. This
 *  menu is currently located in the toolbar.
*/
class UserCalendarPool : public QObject
{
    Q_OBJECT

public:
    UserCalendarPool(QObject* parent = 0);
    ~UserCalendarPool();

    const UserCalendarInfo* addUserCalendar(const QColor & color, const QString & title, const bool visible);
    void addUserCalendarFromStorage(UserCalendarInfo* &info);

    void removeUserCalendar(const int id);
    QColor color(const int id) const;
    QString title(const int id) const;
    bool isVisible(const int id) const;
    const UserCalendarInfo* item(const int id) const;
    void setData(const int id, const QColor & color, const QString & title, const bool visible);

    QMenu* calendarMenu();
    const QList<UserCalendarInfo*> calendarInfos() const;

private:
    int m_nextInsertId;
    QMenu* m_userCalendarMenu;
    QList<UserCalendarInfo*> m_pool;
    void toggleVisibility(const int id);

signals:
    void signalUserCalendarInUseModified();
    void signalUserCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible);

private slots:
    void slotActionTriggered(QAction* action);

};

#endif // CALGROUPENTRY_H
