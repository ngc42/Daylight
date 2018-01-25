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
#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <QSet>

#include "appointmentmanager.h"
#include "usercalendar.h"


namespace Ui {
    class AppointmentDialog;
}


/* AppointmentDialog shows up a dialog, where a user can set up a new appointment or modify an existing one.
 *  the Dialog is startet in the MainWindow.
 * Reconfiguring an appointment involves calling setAppointmentValues().
 * UI-parts of the dialog are in appointmentdialog.ui. */
class AppointmentDialog : public QDialog
{
    Q_OBJECT

    // just for a combobox.
    // this looks like duplicated code, see appointmentmanager.h (Recurrence) for details
    enum RecurrenceFrequencyType {
        NO_RECURRENCE, YEARLY, MONTHLY, WEEKLY, DAILY
    };

public:
    explicit AppointmentDialog( QWidget* parent = Q_NULLPTR );
    ~AppointmentDialog();
    QString appointmentId() const { return m_appointment->m_uid; }
    RecurrenceFrequencyType recurrence() const;
    bool isNewAppointment() const { return m_isNewAppointment; }
    Appointment* appointment() { return m_appointment; }
    void deleteAppointment() { delete m_appointment; }


    void setUpTimezones();

    void reset();
    void reset( const QDate date );
    void resetRecurrencePage();
    void setDefaultBasicInterval( const QDate date );
    void setDefaultBasicInterval( const QDateTime dateTime );

    void setUserCalendarInfos(QList<UserCalendarInfo*> &uciList);
    void setUserCalendarIndexById( const int usercalendarId );

    void createNewAppointment();
    void setAppointmentValues(Appointment* apmData );
    void setTimezoneIndexesByIanaId( const QByteArray iana1, const QByteArray iana2 );

    void collectAppointmentData();

private:
    Ui::AppointmentDialog *m_ui;
    QDateTime   m_dtSaveStart;
    QDateTime   m_dtSaveEnd;

    Appointment*    m_appointment;
    // @fixme: need to notify about changes
    int             m_sequence;
    bool            m_isNewAppointment;

    QSet<int>   m_weeksByWeekNo;    // Recurrence, ByWeekNo, Set of weeks
    QSet<int>   m_daysByYearDay;    // Recurrence, ByYearDays, Set of days
    QSet<int>   m_daysByMonthDay;   // Recurrence, ByYearDays, Set of days
    QSet<int>   m_weekDaysDaysByDay;    // Recurrence, ByDays, <WeekDay * 1000 + 500 + DayNum>
    QSet<int>   m_setPos;           // Recurrence, ByDays, <WeekDay * 1000 + 500 + DayNum>

signals:
private slots:
    // page basic
    void slotIndexChangedRecurrenceFrequency( int index );

    // page recurrence
    void slotAddWeekNoClicked();
    void slotRemoveWeekNoClicked();

    void slotAddYearDayClicked();
    void slotRemoveYearDayClicked();

    void slotAddMonthDayClicked();
    void slotRemoveMonthDayClicked();

    void slotAddDayDayClicked();
    void slotRemoveDayDayClicked();

    void slotAddSetposClicked();
    void slotRemoveSetposClicked();

    // page alarm
};

#endif // APPOINTMENTDIALOG_H
