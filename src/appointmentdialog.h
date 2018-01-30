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
#ifndef APPOINTMENTDIALOG_H
#define APPOINTMENTDIALOG_H

#include "appointmentmanager.h"
#include "usercalendar.h"

#include <QButtonGroup>
#include <QDateTime>
#include <QDialog>
#include <QSet>

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

    /* values for radiobuttons Recurrence/Misc/Repeat.
     * According to standard, UNTIL and COUNT must not be in the same RRULE.
     *  But one of them may occur. */
    enum RepeatRestrictionType {
        REPEAT_FOREVER, REPEAT_UNTIL, REPEAT_COUNT
    };

public:
    explicit AppointmentDialog( QWidget* parent = Q_NULLPTR );

    ~AppointmentDialog();

    QString appointmentId() const { return m_appointment->m_uid; }

    RecurrenceFrequencyType recurrence() const;

    /* If we are modifying an already existing appointment, this method
     *  returns false.
     * The dialog was then startet with setAppointmentValues(ExistingAppointment).
     */
    bool isNewAppointment() const { return m_isNewAppointment; }

    /* Finds out, if the data here differs from m_storedOrigAppointment.
     * This is only applicable, if setAppointmentValues was used to store an
     * used appointment, in this case, m_isNewAppointment is false.
     * - returns false, if user has not modified anything
     * - returns false, if m_isNewAppointment is true
     * - else returns true
     * @fixme: does not check recurrences (partially included)
     * @fixme: does not check alarm
     */
    bool modified() const;

    Appointment* appointment() { return m_appointment; }

    /* Deletes a newly created appointment. Used, if this dialog is cancelled or an
     * in-use appointment wasn't modified
     */
    void deleteAppointment();

    // timezones as strings in the form.
    void setUpTimezones();

    // reset default values
    void reset();
    void reset( const QDate date );
    void resetRecurrencePage();
    void setDefaultBasicInterval( const QDate date );
    void setDefaultBasicInterval( const QDateTime dateTime );

    // setting user calendars
    void setUserCalendarInfos( QList<UserCalendarInfo*> &uciList );
    void setUserCalendarIndexById( const int usercalendarId );

    /* We can create a new appointment - this is done by clicking on a free
     *  day.
     * We can also modify an existing appointment by clicking on an appointment
     */
    void createNewAppointment();
    void setAppointmentValues( const Appointment* apmData );

    // modified in-use-appointment, increse sequence number
    void increaseSequence();

    // set up timezone-Combos with existing iana ids.
    void setTimezoneIndexesByIanaId( const QByteArray iana1, const QByteArray iana2 );

    /* we are close to finish: Here, we collect the user input from the basic page.
     * @fixme: missing m_sequence, use modified() for it.
     */
    void collectAppointmentDataFromBasicPage();

private:
    Ui::AppointmentDialog*  m_ui;
    QButtonGroup*           m_repeatByMonthButtonGroup;
    QButtonGroup*           m_repeatRestrictionButtonGroup;

    QDateTime               m_dtSaveStart;
    QDateTime               m_dtSaveEnd;

    // the new-to-create aappointment
    Appointment*        m_appointment;

    // Original appointment to compare to m_appointment
    // used to calculate m_sequence
    // Set by setAppointmentValues
    const Appointment*  m_storedOrigAppointment;

    // false, if we modify an existing appointment
    bool        m_isNewAppointment;

    RepeatRestrictionType repeatRestriction() const;

signals:
private slots:
    // page basic
    void slotIndexChangedRecurrenceFrequency( int index );

    // page recurrence
    void slotMonthClicked( int id, bool checked );

    void slotAddWeekNoClicked();
    void slotRemoveWeekNoClicked();

    void slotAddYearDayClicked();
    void slotRemoveYearDayClicked();

    void slotAddMonthDayClicked();
    void slotRemoveMonthDayClicked();

    void slotAddDayDayClicked();
    void slotRemoveDayDayClicked();

    void slotRepeatRestrictionRadiobuttonClicked(int id);

    void slotAddSetposClicked();
    void slotRemoveSetposClicked();

    // page alarm
};

#endif // APPOINTMENTDIALOG_H
