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
#include "storage.h"
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


Storage::Storage(QObject *parent) :
    QObject(parent)
{
    createDatabase();
}


void Storage::createDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("daylightdb.sqlite3");
    bool ok = m_db.open();
    qDebug() << "OPEN DATABASE --> " << ok;

    QSqlQuery query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS usercalendars(id INT, title VARCHAR, redcolor INT, greencolor INT, bluecolor INT, visible BOOL)");
    QSqlError err = query.lastError();
    qDebug() << " CREATE TABLE usercalendars ok:" << (err.type() == QSqlError::NoError ) << " err-text" << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS appointments "
             "(appointment_id INT, usercalendar_id INT, allday BOOL, title VARCHAR, startdt DATETIME, enddt DATETIME)");
    err = query.lastError();
    qDebug() << " CREATE TABLE appointments ok:" << (err.type() == QSqlError::NoError ) << " err-text" << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS recurrences(appointment_id INT, rectype INT, forever BOOL, lastdt DATETIME)");
    err = query.lastError();
    qDebug() << " CREATE TABLE recurrences ok:" << (err.type() == QSqlError::NoError ) << " err-text" << err.text();

}


void Storage::loadAppointmentData()
{
    QSqlQuery qApmSelect("SELECT appointment_id, usercalendar_id, allday, title, startdt, enddt FROM appointments", m_db);
    QSqlQuery qRecSelect(m_db);
    qRecSelect.prepare("SELECT appointment_id, rectype, forever, lastdt FROM recurrences WHERE appointment_id=:id");
    if( qApmSelect.exec() )
    {
        bool ok;
        while (qApmSelect.next())
        {            
            AppointmentData apmData;
            apmData.m_appointmentId = qApmSelect.value(0).toInt(&ok);
            apmData.m_userCalendarId = qApmSelect.value(1).toInt(&ok);
            apmData.m_allDay = qApmSelect.value(2).toBool();
            apmData.m_title = qApmSelect.value(3).toString();
            apmData.m_startDt = qApmSelect.value(4).toDateTime();
            apmData.m_endDt = qApmSelect.value(5).toDateTime();

            RecurrenceData recData;
            recData.m_appointmentId = apmData.m_appointmentId;
            qRecSelect.bindValue(":id", apmData.m_appointmentId);
            if( !qRecSelect.exec() )
            {
                qDebug() << "ERR - no recurrence for id in Storage::loadAppointmentData() - create an empty one";
                qDebug() << " - Error is:" << qRecSelect.lastError().text();
                recData.m_type = RecurrenceType::R_NO_RECURRENCE;
                recData.m_forever = false;
                recData.m_lastDt = QDateTime::currentDateTime();
            }
            else
            {
                qRecSelect.first();
                recData.m_type = (RecurrenceType) qRecSelect.value(1).toInt(&ok);
                recData.m_forever = qRecSelect.value(2).toBool();
                recData.m_lastDt = qRecSelect.value(3).toDateTime();
            }
            emit signalLoadedAppointmentFromStorage(apmData, recData);
        }
    }
    else
        qDebug() << "Storage::loadAppointmentData() Query Error: " << qApmSelect.lastError().text();
}


void Storage::modifyAppointment(const AppointmentData & apmData, const RecurrenceData & recData)
{
    if(apmData.m_appointmentId != recData.m_appointmentId)
    {
        qDebug() << "ERR: Storage::slotAppointmentModify(): id mismatch";
        return;
    }

    QSqlQuery qApm(m_db);
    qApm.prepare("UPDATE appointments SET usercalendar_id=:ucid, allday=:allday, title=:title, startdt=:startdt, enddt=:enddt WHERE appointment_id=:id");
    qApm.bindValue(":ucid", apmData.m_userCalendarId);
    qApm.bindValue(":allday", apmData.m_allDay);
    qApm.bindValue(":title", apmData.m_title);
    qApm.bindValue(":startdt", apmData.m_startDt);
    qApm.bindValue(":enddt", apmData.m_endDt);
    qApm.bindValue(":id", apmData.m_appointmentId);

    QSqlQuery qRec(m_db);
    qRec.prepare("UPDATE recurrences SET rectype=:rectype, forever=:forever, lastdt=:lastdt WHERE appointment_id=:id");
    qRec.bindValue(":rectype", recData.m_type);
    qRec.bindValue(":forever", recData.m_forever);
    qRec.bindValue(":lastdt", recData.m_lastDt);
    qRec.bindValue(":id", recData.m_appointmentId);

    if( m_db.transaction() )
    {
        qApm.exec();
        qRec.exec();
        m_db.commit();
    }
    else
    {
        qApm.exec();
        qRec.exec();
    }
}


void Storage::setAppointmentsCalendar(const int appointmentId, const int calendarId)
{
    QSqlQuery qUpdate(m_db);
    qUpdate.prepare("UPDATE appointments SET usercalendar_id=:calid  WHERE appointment_id=:id");
    qUpdate.bindValue(":calid", calendarId);
    qUpdate.bindValue(":id", appointmentId);
    if(! qUpdate.exec())
    {
        qDebug() << "ERR: Storage::setAppointmentsCalendar():" << qUpdate.lastError().text();
    }
}


void Storage::removeAppointment(const int id)
{
    QSqlQuery qApm(m_db);
    qApm.prepare("DELETE FROM appointments WHERE appointment_id=:id");
    qApm.bindValue(":id", id);

    QSqlQuery qRec(m_db);
    qRec.prepare("DELETE FROM recurrences WHERE appointment_id=:id");
    qRec.bindValue(":id", id);

    if( m_db.transaction() )
    {
        qApm.exec();
        qRec.exec();
        m_db.commit();
    }
    else
    {
        qApm.exec();
        qRec.exec();
    }
}


/* Load calendar info and tell it to the outside world by signal */
void Storage::loadUserCalendarInfo()
{
    QSqlQuery query("SELECT id, title, redcolor, greencolor, bluecolor, visible FROM usercalendars", m_db);
    if( query.exec() )
    {
        bool ok;
        while (query.next())
        {
            UserCalendarInfo* info = new UserCalendarInfo;
            info->m_id = query.value(0).toInt(&ok);
            info->m_title = query.value(1).toString();
            int red = query.value(2).toInt(&ok);
            int green = query.value(3).toInt(&ok);
            int blue = query.value(4).toInt(&ok);
            QColor c(red, green, blue);
            info->m_color = c;
            info->m_isVisible = query.value(5).toBool();
            emit signalLoadedUserCalendarFromStorage(info);
        }
    }
    else
        qDebug() << " Query Error: " << query.lastError().text();
}


/* Insert UserCalendarInfo into database.
 * duplicates in db (by id) are deleted */
void Storage::insertUserCalendarInfo(const UserCalendarInfo* ucinfo)
{
    if( m_db.isOpen() )
    {
        QSqlQuery query_del(m_db), query_insert(m_db);
        query_del.prepare("DELETE FROM usercalendars WHERE id=:id");
        query_del.bindValue(":id", ucinfo->m_id);

        query_insert.prepare
                ("INSERT INTO usercalendars VALUES(:id, :title, :red, :green, :blue, :visible)");
        query_insert.bindValue(":id", ucinfo->m_id);
        QString title = ucinfo->m_title;
        title.truncate(49);
        query_insert.bindValue(":title", title);
        query_insert.bindValue(":red", ucinfo->m_color.red());
        query_insert.bindValue(":green", ucinfo->m_color.green());
        query_insert.bindValue(":blue", ucinfo->m_color.blue());
        query_insert.bindValue(":visible", ucinfo->m_isVisible);

        if( m_db.transaction() )
        {
            query_del.exec();
            query_insert.exec();
            m_db.commit();
        }
        else
        {
            query_del.exec();
            query_insert.exec();
        }
    }
    else
        qDebug() << " ERR: Storage::insertUserCalendarInfo()  - db not open";
}


/* delete calendar and associated recurrences and appointments */
void Storage::removeUserCalendar(const int id)
{
    QSqlQuery qApmSelect(m_db);
    qApmSelect.prepare("SELECT appointment_id, usercalendar_id FROM appointments WHERE usercalendar_id=:id");
    QSqlQuery qRecDelete(m_db);
    qRecDelete.prepare("DELETE FROM recurrences WHERE appointment_id=:appid");
    QSqlQuery qApmDelete(m_db);
    qApmDelete.prepare("DELETE FROM appointments WHERE usercalendar_id=:id");
    QSqlQuery qCalDelete(m_db);
    qCalDelete.prepare("DELETE FROM usercalendars WHERE id=:id");

    // 1. Delete recurrences based on appointments with calendar id
    qApmSelect.bindValue(":id", id);
    if( qApmSelect.exec() )
    {
        bool ok;
        while (qApmSelect.next())
        {
            int appId = qApmSelect.value(0).toInt(&ok);
            qRecDelete.bindValue(":appid", appId);
            if(! qRecDelete.exec() )
            {
                qDebug() << "ERR: Storage::removeUserCalendar(), qRecDelete, " << qRecDelete.lastError().text();
            }
        }
    }
    else
    {
        qDebug() << "ERR: Storage::removeUserCalendar(), qApmSelect, " << qApmSelect.lastError().text();
    }
    // 2. delete appointments
    qApmDelete.bindValue(":id", id);
    if(! qApmDelete.exec() )
    {
        qDebug() << "ERR: Storage::removeUserCalendar(), qApmDelete, " << qApmDelete.lastError().text();
    }
    // 3. delete user calendar
    qCalDelete.bindValue(":id", id);
    if(! qCalDelete.exec() )
    {
        qDebug() << "ERR: Storage::removeUserCalendar(), qCalDelete, " << qCalDelete.lastError().text();
    }
}


void Storage::slotAppointmentAdd(const AppointmentData & apmData, const RecurrenceData & recData)
{
    if(apmData.m_appointmentId != recData.m_appointmentId)
    {
        qDebug() << "ERR: Storage::slotAppointmentAdd(): id mismatch";
        return;
    }

    QSqlQuery qApm(m_db);
    qApm.prepare("INSERT INTO appointments VALUES(:id, :ucid, :allday, :title, :startdt, :enddt)");
    qApm.bindValue(":id", apmData.m_appointmentId);
    qApm.bindValue(":ucid", apmData.m_userCalendarId);
    qApm.bindValue(":allday", apmData.m_allDay);
    qApm.bindValue(":title", apmData.m_title);
    qApm.bindValue(":startdt", apmData.m_startDt);
    qApm.bindValue(":enddt", apmData.m_endDt);

    QSqlQuery qRec(m_db);
    qRec.prepare("INSERT INTO recurrences VALUES(:id, :rectype, :forever, :lastdt)");
    qRec.bindValue(":id", recData.m_appointmentId);
    qRec.bindValue(":rectype", recData.m_type);
    qRec.bindValue(":forever", recData.m_forever);
    qRec.bindValue(":lastdt", recData.m_lastDt);

    if( m_db.transaction() )
    {
        qApm.exec();
        qRec.exec();
        m_db.commit();
    }
    else
    {
        qApm.exec();
        qRec.exec();
    }
}


void Storage::slotUserCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible)
{
    QSqlQuery qUpdate(m_db);
    qUpdate.prepare("UPDATE usercalendars SET title=:title, redcolor=:red, greencolor=:green, bluecolor=:blue, visible=:visible WHERE id=:id");
    qUpdate.bindValue(":title", title);
    qUpdate.bindValue(":red", color.red());
    qUpdate.bindValue(":green", color.green());
    qUpdate.bindValue(":blue", color.blue());
    qUpdate.bindValue(":visible", visible);
    qUpdate.bindValue(":id", id);
    if(! qUpdate.exec())
    {
        qDebug() << " UPDATE Error: " << qUpdate.lastError().text();
    }
}

