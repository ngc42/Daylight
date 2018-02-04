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
#include <QDebug>
#include <QSqlError>
#include <QSqlQuery>

#include "datetime.h"
#include "storage.h"


Storage::Storage()
{
    createDatabase();
}


void Storage::createDatabase()
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName("daylightdb.sqlite3");
    bool ok = m_db.open();
    qDebug() << "OPEN DATABASE --> " << ok;

    /* @fixme: test, if table version exists
     * select count(name) from sqlite_master where name ='version';
     * Yes: check version
     *  - same_version: ok.
     *  - different_version: convert.
     * No: create and write version
     *
     * Maybe, QSqlDatabase::tables() works for this
     */

    QSqlQuery query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS usercalendars"
             "(id INT, title VARCHAR, "
             "redcolor INT, greencolor INT, bluecolor INT, visible BOOL)");
    QSqlError err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE usercalendars : " << err.text();

    QSqlQuery test_if_exists = m_db.exec( "select count(id) from usercalendars where id=0");
    if( test_if_exists.first() )
    {
        bool ok;
        int numDefaultIds = test_if_exists.value(0).toInt(&ok);
        if( numDefaultIds < 1 )
            m_db.exec( "INSERT INTO usercalendars VALUES(0, 'DEFAULT', 200, 50, 50, 1)");
    }
    else
        qDebug() << "ERROR: Storage::createDatabase(): Cannot determine number of usercalendars with default id";


    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS basics"
             "(uid VARCHAR, sequence INT, "
             "start DATETIME, start_tz VARCHAR, end DATETIME, end_tz VARCHAR,"
             "summary VARCHAR, description VARCHAR, busyfree INT)");
    err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE basics : " << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS alarms"
             "(uid VARCHAR, rel_timeout BIGINT, "
             "repeats INT, pause_between BIGINT)");
    err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE alarms : " << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS recurrences"
             "(uid VARCHAR, frequency INT, count INT, interval INT,"
             "until DATETIME, until_tz VARCHAR,"
             "start_wd INT,"
             "exdates VARCHAR, exdates_tz VARCHAR,"
             "fixeddates VARCHAR, fixeddates_tz VARCHAR,"
             "bymonthlist VARCHAR, byweeknumberlist VARCHAR, byyeardaylist VARCHAR,"
             "bymonthdaylist VACHAR, bydaymap VARCHAR,"
             "byhourlist VARCHAR, byminutelist VARCHAR, bysecondlist VARCHAR,"
             "bysetposlist VARCHAR)");
    err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE recurrences : " << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS events"
             "(uid VARCHAR, text VARCHAR, start DATETIME, end DATETIME,"
             "timezone VARCHAR, is_alarm BOOL)");
    err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE events : " << err.text();

    query = m_db.exec
            ("CREATE TABLE IF NOT EXISTS appointments"
             "(uid VARCHAR, min_year INT, max_year INT, allyears VARCHAR, "
             "usercalendar_id INT,"
             "have_recurrence BOOL, have_alarms BOOL)");
    err = query.lastError();
    if( err.type() != QSqlError::NoError )
        qDebug() << " ERROR: Storage::createDatabase(): CREATE TABLE appointments : " << err.text();
}


void Storage::storeAppointment( const Appointment &apmData )
{
    QSqlQuery iApm(m_db);
    iApm.prepare("INSERT INTO appointments VALUES(:uid, :minyear, :maxyear, :allyears, :calid, :haverec, :havealarm)");
    iApm.bindValue(":uid", apmData.m_uid);
    iApm.bindValue(":minyear", apmData.m_minYear);
    iApm.bindValue(":maxyear", apmData.m_maxYear);
    QString intSetString;
    Appointment::makeStringFromIntSet( apmData.m_yearsInQuestion, intSetString );
    iApm.bindValue(":allyears", intSetString );
    iApm.bindValue(":calid", apmData.m_userCalendarId );
    iApm.bindValue(":haverec", apmData.m_haveRecurrence );
    iApm.bindValue(":havealarm", apmData.m_haveAlarm );
    iApm.exec();

    QSqlQuery iBas(m_db);
    iBas.prepare("INSERT INTO basics VALUES(:uid, :sequence, :start, :starttz, :end, :endtz, :summary, :description, :busyfree)");
    iBas.bindValue(":uid", apmData.m_uid);
    iBas.bindValue(":sequence", apmData.m_appBasics->m_sequence);
    QString dtString;
    QString tzString;
    DateTime::dateTime2Strings( apmData.m_appBasics->m_dtStart, dtString, tzString );
    iBas.bindValue(":start", dtString );
    iBas.bindValue(":starttz", tzString );
    DateTime::dateTime2Strings( apmData.m_appBasics->m_dtEnd, dtString, tzString );
    iBas.bindValue(":end", dtString );
    iBas.bindValue(":endtz", tzString );
    iBas.bindValue(":summary", apmData.m_appBasics->m_summary );
    iBas.bindValue(":description", apmData.m_appBasics->m_description );
    iBas.bindValue(":busyfree", static_cast<int>(apmData.m_appBasics->m_busyFree) );
    iBas.exec();

    for( const AppointmentAlarm* alarm : apmData.m_appAlarms )
    {
        QSqlQuery iAla(m_db);
        iAla.prepare("INSERT INTO alarms VALUES(:uid, :reltmout, :repeat, :pause)");
        iAla.bindValue(":uid", apmData.m_uid );
        iAla.bindValue(":reltmout", alarm->m_alarmSecs );
        iAla.bindValue(":repeat", alarm->m_repeatNumber );
        iAla.bindValue(":pause", alarm->m_pauseSecs );
        iAla.exec();
    }

    if( apmData.m_haveRecurrence )
    {
        QSqlQuery iRec(m_db);
        iRec.prepare("INSERT INTO recurrences VALUES(:uid, :frequency, :count, :interval, :until, :untiltz, "
                     ":startwd, :exdates, :exdatestz, :fixeddates, :fixeddatestz, "
                     ":bymonthlist, :bywnlist, :byydlist, :bymdlist, :bydmap,"
                     ":byhlist, :bymlist, :byslist, :bysetposlist)");
        iRec.bindValue(":uid", apmData.m_uid);
        iRec.bindValue(":frequency", static_cast<int>(apmData.m_appRecurrence->m_frequency) );
        iRec.bindValue(":count", apmData.m_appRecurrence->m_count );
        iRec.bindValue(":interval", apmData.m_appRecurrence->m_interval );
        DateTime::dateTime2Strings( apmData.m_appRecurrence->m_until, dtString, tzString );
        iRec.bindValue(":until", dtString );
        iRec.bindValue(":untiltz", tzString );
        iRec.bindValue(":startwd", static_cast<int>(apmData.m_appRecurrence->m_startWeekday) );
        Appointment::makeStringsFromDateVector( apmData.m_appRecurrence->m_exceptionDates, dtString, tzString );
        iRec.bindValue(":exdates", dtString );
        iRec.bindValue(":exdatestz", tzString );
        Appointment::makeStringsFromDateVector( apmData.m_appRecurrence->m_fixedDates, dtString, tzString );
        iRec.bindValue(":fixeddates", dtString );
        iRec.bindValue(":fixeddatestz", tzString );
        QString listString;
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byMonthSet, listString );
        iRec.bindValue(":bymonthlist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byWeekNumberSet, listString );
        iRec.bindValue(":bywnlist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byYearDaySet, listString );
        iRec.bindValue(":byydlist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byMonthDaySet, listString );
        iRec.bindValue(":bymdlist", listString );
        Appointment::makeStringFromDayset( apmData.m_appRecurrence->m_byDaySet, listString );
        iRec.bindValue(":bydmap", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byHourSet, listString );
        iRec.bindValue(":byhlist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_byMinuteSet, listString );
        iRec.bindValue(":bymlist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_bySecondSet, listString );
        iRec.bindValue(":byslist", listString );
        Appointment::makeStringFromIntSet( apmData.m_appRecurrence->m_bySetPosSet, listString );
        iRec.bindValue(":bysetposlist", listString );
        iRec.exec();
    }

    m_db.transaction();
    QSqlQuery iEve(m_db);
    int countAppointments = apmData.m_eventVector.count() - 1;
    int currentCount = 0;
    for( const Event e : apmData.m_eventVector )
    {
        iEve.prepare("INSERT INTO events VALUES(:uid, :text, :start, :end, :timezone, :is_alarm)");
        iEve.bindValue(":uid", apmData.m_uid);
        iEve.bindValue(":text", e.m_displayText );
        DateTime::dateTime2Strings( e.m_startDt, dtString, tzString );
        iEve.bindValue(":start", dtString );
        DateTime::dateTime2Strings( e.m_endDt, dtString, tzString );
        iEve.bindValue(":end", dtString );
        iEve.bindValue(":timezone", tzString );
        iEve.bindValue(":is_alarm", e.m_isAlarmEvent );
        iEve.exec();
        emit sigStoreEvent(0, currentCount++, countAppointments );
    }
    m_db.commit();
}


void Storage::updateAppointment( const Appointment &apmData )
{
    if( not apmData.m_uid.isEmpty() )
    {
        removeAppointment( apmData.m_uid );
        storeAppointment( apmData );
    }
}


void Storage::loadAppointmentByYear(const int year, QVector<Appointment*>& outAppointments )
{
    outAppointments.clear();

    QSqlQuery qApmSelect( m_db );
    qApmSelect.prepare( "SELECT uid, min_year, max_year, allyears, "
                        "usercalendar_id, have_recurrence, have_alarms "
                        "FROM appointments WHERE min_year <= :mi and max_year >= :ma" );
    qApmSelect.bindValue( ":mi", year );
    qApmSelect.bindValue( ":ma", year );

    QSqlQuery qApmBasics( m_db );
    qApmBasics.prepare( "SELECT uid, sequence, start, start_tz, end, end_tz,"
                        "summary, description, busyfree "
                        "FROM basics WHERE uid=:puid" );

    QSqlQuery qApmAlarm( m_db );
    qApmAlarm.prepare( "SELECT uid, rel_timeout, repeats, pause_between "
                        "FROM alarms WHERE uid=:puid" );

    QSqlQuery qApmRecurrence( m_db );
    qApmRecurrence.prepare( "SELECT uid, frequency, count, interval,"
                            "until, until_tz,"
                            "start_wd, exdates, exdates_tz,"
                            "fixeddates, fixeddates_tz,"
                            "bymonthlist, byweeknumberlist, byyeardaylist,"
                            "bymonthdaylist, bydaymap, byhourlist, "
                            "byminutelist, bysecondlist, bysetposlist "
                            "FROM recurrences WHERE uid=:puid");

    QSqlQuery qApmEvents( m_db );
    qApmEvents.prepare( "SELECT uid, text, start, end,"
                        "timezone, is_alarm "
                        "FROM events WHERE uid=:puid");

    if( qApmSelect.exec() )
    {
        bool ok;
        while (qApmSelect.next())
        {
            // read Appointment
            Appointment *apmData = new Appointment();
            apmData->m_uid       = qApmSelect.value(0).toString();
            apmData->m_minYear   = qApmSelect.value(1).toString().toInt( &ok );
            apmData->m_maxYear   = qApmSelect.value(2).toString().toInt( &ok );
            QString yearsString = qApmSelect.value(3).toString();
            for( const QString s : yearsString.split( ',', QString::SkipEmptyParts) )
                apmData->m_yearsInQuestion.insert( s.toInt(&ok) );
            apmData->m_userCalendarId    = qApmSelect.value(4).toString().toInt(&ok);
            apmData->m_haveRecurrence    = qApmSelect.value(5).toBool();
            apmData->m_haveAlarm         = qApmSelect.value(6).toBool();

            // read AppointmentBasic
            qApmBasics.bindValue( ":puid", apmData->m_uid );
            if( not qApmBasics.exec() )
            {
                qDebug() << "ERR qApmBasics.exec()";
                return;
            }
            qApmBasics.first();
            AppointmentBasics* apmBasic = new AppointmentBasics();
            apmBasic->m_uid      = qApmBasics.value(0).toString();
            apmBasic->m_sequence = qApmBasics.value(1).toString().toInt(&ok);
            apmBasic->m_dtStart  = DateTime::string2DateTime( qApmBasics.value(2).toString(), qApmBasics.value(3).toString() );
            apmBasic->m_dtEnd    = DateTime::string2DateTime( qApmBasics.value(4).toString(), qApmBasics.value(5).toString() );
            apmBasic->m_summary  = qApmBasics.value(6).toString();
            apmBasic->m_description  = qApmBasics.value(7).toString();
            apmBasic->m_busyFree     = static_cast<AppointmentBasics::BusyFreeType>(qApmBasics.value(8).toString().toInt(&ok));
            apmData->m_appBasics = apmBasic;

            // read Alarms
            if( apmData->m_haveAlarm )
            {
                qApmAlarm.bindValue( ":puid", apmData->m_uid );
                if( not qApmAlarm.exec() )
                {
                    qDebug() << "ERR qApmAlarm.exec()";
                    return;
                }
                while( qApmAlarm.next() )
                {
                    AppointmentAlarm* alarm = new AppointmentAlarm();
                    alarm->m_alarmSecs      = qApmAlarm.value(1).toLongLong(&ok);
                    alarm->m_repeatNumber   = qApmAlarm.value(2).toInt(&ok);
                    alarm->m_pauseSecs      = qApmAlarm.value(3).toLongLong(&ok);
                    apmData->m_appAlarms.append(alarm);
                }
            }

            // read Recurrences
            if( apmData->m_haveRecurrence )
            {
                qApmRecurrence.bindValue( ":puid", apmData->m_uid );
                if( not qApmRecurrence.exec() )
                {
                    qDebug() << "ERR qApmRecurrence.exec()";
                    return;
                }

                qApmRecurrence.first();
                AppointmentRecurrence* apmRecurrence = new AppointmentRecurrence();

                apmRecurrence->m_frequency  = static_cast<AppointmentRecurrence::RecurrenceFrequencyType>(qApmRecurrence.value(1).toInt(&ok));
                apmRecurrence->m_count      = qApmRecurrence.value(2).toString().toInt(&ok);
                apmRecurrence->m_interval   = qApmRecurrence.value(3).toString().toInt(&ok);
                apmRecurrence->m_until      = DateTime::string2DateTime( qApmRecurrence.value(4).toString(), qApmRecurrence.value(5).toString());

                // max one of them is true
                apmRecurrence->m_haveCount = false;
                apmRecurrence->m_haveUntil = false;
                if( apmRecurrence->m_count > 0 )
                    apmRecurrence->m_haveCount = true;
                else if( apmRecurrence->m_until.isValid() )
                    apmRecurrence->m_haveUntil = true;

                apmRecurrence->m_startWeekday = static_cast<AppointmentRecurrence::WeekDay>(qApmRecurrence.value(6).toInt(&ok));
                Appointment::makeDateVector( qApmRecurrence.value(7).toString(),
                                                     qApmRecurrence.value(8).toString(),
                                                     apmRecurrence->m_exceptionDates );
                Appointment::makeDateVector( qApmRecurrence.value(9).toString(),
                                                     qApmRecurrence.value(10).toString(),
                                                     apmRecurrence->m_fixedDates );
                Appointment::makeIntSet( qApmRecurrence.value(11).toString(), apmRecurrence->m_byMonthSet );
                Appointment::makeIntSet( qApmRecurrence.value(12).toString(), apmRecurrence->m_byWeekNumberSet );
                Appointment::makeIntSet( qApmRecurrence.value(13).toString(), apmRecurrence->m_byYearDaySet );
                Appointment::makeIntSet( qApmRecurrence.value(14).toString(), apmRecurrence->m_byMonthDaySet );
                Appointment::makeDayset( qApmRecurrence.value(15).toString(), apmRecurrence->m_byDaySet );
                Appointment::makeIntSet( qApmRecurrence.value(16).toString(), apmRecurrence->m_byHourSet );
                Appointment::makeIntSet( qApmRecurrence.value(17).toString(), apmRecurrence->m_byMinuteSet );
                Appointment::makeIntSet( qApmRecurrence.value(18).toString(), apmRecurrence->m_bySecondSet );
                Appointment::makeIntSet( qApmRecurrence.value(19).toString(), apmRecurrence->m_bySetPosSet );
                apmData->m_appRecurrence = apmRecurrence;
            }

            // read Events
            qApmEvents.bindValue( ":puid", apmData->m_uid );
            if( not qApmEvents.exec() )
            {
                qDebug() << "ERR qApmEvents.exec()";
                return;
            }
            while (qApmEvents.next())
            {
                Event e;
                e.m_uid         = qApmEvents.value(0).toString();
                e.m_displayText = qApmEvents.value(1).toString();
                QString tzString    = qApmEvents.value(4).toString();
                e.m_startDt     = DateTime::string2DateTime( qApmEvents.value(2).toString(), tzString );
                e.m_endDt       = DateTime::string2DateTime( qApmEvents.value(3).toString(), tzString );
                e.m_isAlarmEvent    = qApmEvents.value(5).toBool();
                e.m_userCalendarId = apmData->m_userCalendarId;
                apmData->m_eventVector.append( e );
            }

            outAppointments.append( std::move(apmData) );
        }
    }
    else
        qDebug() << "Storage::loadAppointmentData() Query Error: " << qApmSelect.lastError().text();
}


void Storage::removeAppointment( const QString id )
{
    QSqlQuery qApm(m_db);
    qApm.prepare("DELETE FROM appointments WHERE uid=:id");
    qApm.bindValue(":id", id);
    QSqlQuery qBas(m_db);
    qBas.prepare("DELETE FROM basics WHERE uid=:id");
    qBas.bindValue(":id", id);
    QSqlQuery qAla(m_db);
    qAla.prepare("DELETE FROM alarms WHERE uid=:id");
    qAla.bindValue(":id", id);
    QSqlQuery qRec(m_db);
    qRec.prepare("DELETE FROM recurrences WHERE uid=:id");
    qRec.bindValue(":id", id);
    QSqlQuery qEve(m_db);
    qEve.prepare("DELETE FROM events WHERE uid=:id");
    qEve.bindValue(":id", id);

    if( m_db.transaction() )
    {
        qApm.exec();
        qBas.exec();
        qAla.exec();
        qRec.exec();
        qEve.exec();
        m_db.commit();
    }
    else
    {
        qApm.exec();
        qBas.exec();
        qAla.exec();
        qRec.exec();
        qEve.exec();
    }
}


void Storage::setAppointmentsCalendar(const QString appointmentId, const int calendarId)
{
    QSqlQuery qUpdate(m_db);
    qUpdate.prepare("UPDATE appointments SET usercalendar_id=:calid  WHERE uid=:id");
    qUpdate.bindValue(":calid", calendarId);
    qUpdate.bindValue(":id", appointmentId);
    if(! qUpdate.exec())
    {
        qDebug() << "ERR: Storage::setAppointmentsCalendar():" << qUpdate.lastError().text();
    }
}


/* Load calendar info and tell it to the outside world by signal */
void Storage::loadUserCalendarInfo(UserCalendarPool*& ucalPool)
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
            ucalPool->addUserCalendarFromStorage( info );
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


void Storage::userCalendarDataModified(const int id, const QColor & color, const QString & title, const bool visible)
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


/* delete calendar and associated recurrences and appointments */
void Storage::removeUserCalendar(const int id)
{
    QSqlQuery qApmSelect(m_db);
    qApmSelect.prepare("SELECT uid, usercalendar_id FROM appointments WHERE usercalendar_id=:id");

    QSqlQuery qUcalDelete(m_db);
    qUcalDelete.prepare("DELETE FROM usercalendars WHERE id=:id");

    // Delete appointments
    qApmSelect.bindValue(":id", id);
    if( qApmSelect.exec() )
    {
        while (qApmSelect.next())
        {
            QString uid = qApmSelect.value(0).toString();
            removeAppointment( uid );
        }
    }
    else
        qDebug() << "ERR: Storage::removeUserCalendar(), qApmSelect, " << qApmSelect.lastError().text();

    // 3. delete user calendar
    qUcalDelete.bindValue(":id", id);
    if(not qUcalDelete.exec() )
        qDebug() << "ERR: Storage::removeUserCalendar(), qCalDelete, " << qUcalDelete.lastError().text();
}





