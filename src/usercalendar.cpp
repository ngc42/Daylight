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
#include "usercalendar.h"
#include <QDebug>


/***********************************************************
 ******** UserCalendarPool *********************************
***********************************************************/

UserCalendarPool::UserCalendarPool(QObject* parent) :
    QObject(parent), m_nextInsertId(100)
{
    m_userCalendarMenu = new QMenu("Calendars");
    connect(m_userCalendarMenu, SIGNAL(triggered(QAction*)), this, SLOT(slotActionTriggered(QAction*)));
}


UserCalendarPool::~UserCalendarPool()
{
    m_userCalendarMenu->clear();
    for(UserCalendarInfo* uci : m_pool)
    {
        m_pool.removeOne(uci);
        delete uci;
    }
    delete m_userCalendarMenu;
}


const UserCalendarInfo* UserCalendarPool::addUserCalendar(const QColor & color, const QString & title, const bool visible)
{
    UserCalendarInfo* info = new UserCalendarInfo;
    info->m_id = m_nextInsertId;
    info->m_color = color;
    info->m_title = title;
    info->m_isVisible = visible;
    QPixmap pix(8, 8);
    pix.fill(info->m_color);
    info->m_action = new QAction(pix, info->m_title, this);
    info->m_action->setData(info->m_id);
    info->m_action->setCheckable(true);
    info->m_action->setChecked(true);
    info->m_action->setIconVisibleInMenu(true);

    m_pool.append(info);
    m_nextInsertId++;
    return info;
}


void UserCalendarPool::addUserCalendarFromStorage(UserCalendarInfo* & info)
{
    m_nextInsertId = info->m_id >= m_nextInsertId ? info->m_id + 1 : m_nextInsertId;
    QPixmap pix(8, 8);
    pix.fill(info->m_color);
    info->m_action = new QAction(pix, info->m_title, this);
    info->m_action->setData(info->m_id);
    info->m_action->setCheckable(true);
    info->m_action->setChecked(info->m_isVisible);
    info->m_action->setIconVisibleInMenu(true);
    m_pool.append(info);
}


void UserCalendarPool::removeUserCalendar(const int id)
{
    for(UserCalendarInfo* uci : m_pool)
    {
        if(uci->m_id == id)
        {
            m_pool.removeOne(uci);
            delete uci;
        }
    }
}


QColor UserCalendarPool::color(const int id) const
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
            return uci->m_color;
    return Qt::red;
}


QString UserCalendarPool::title(const int id) const
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
            return uci->m_title;
    return QString();
}


bool UserCalendarPool::isVisible(const int id) const
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
            return uci->m_isVisible;
    return true;
}


const UserCalendarInfo* UserCalendarPool::item(const int id) const
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
            return uci;
    return nullptr;
}


void UserCalendarPool::setData(const int id, const QColor & color, const QString & title, const bool visible)
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
        {
            uci->m_color = color;
            uci->m_title = title;
            uci->m_isVisible = visible;
        }
}


QMenu* UserCalendarPool::calendarMenu()
{
    m_userCalendarMenu->clear();
    for(UserCalendarInfo* uci : m_pool)
        m_userCalendarMenu->addAction(uci->m_action);
    return m_userCalendarMenu;
}


const QList<UserCalendarInfo*> UserCalendarPool::calendarInfos() const
{
    return m_pool;
}


void UserCalendarPool::toggleVisibility(const int id)
{
    for(UserCalendarInfo* uci : m_pool)
        if(uci->m_id == id)
            uci->m_isVisible = ! uci->m_isVisible;
}


void UserCalendarPool::slotActionTriggered(QAction* action)
{
    bool ok;
    int id = action->data().toInt(&ok);
    toggleVisibility(id);
    // report that display may change
    emit signalUserCalendarInUseModified();
    // user cal changed and storage wants to know
    const UserCalendarInfo* uci = item(id);
    emit signalUserCalendarDataModified(id, uci->m_color, uci->m_title, uci->m_isVisible);
}
