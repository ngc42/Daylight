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

public:
    explicit AppointmentDialog( QWidget* parent = Q_NULLPTR );
    ~AppointmentDialog();
    QString appointmentId() const { return m_appointment->m_uid; }

    void reset();
    void reset( const QDate date );
    void setDefaultBasicInterval( const QDate date );
    void setDefaultBasicInterval( const QDateTime dateTime );

    void setUserCalendarInfos(QList<UserCalendarInfo*> &uciList);
    void setAppointmentValues(Appointment* apmData );

private:
    Ui::AppointmentDialog *m_ui;
    // saving the datetime for slotWholeDayChanged()
    QDateTime m_dtSaveStart;
    QDateTime m_dtSaveEnd;

    Appointment* m_appointment;


signals:
private slots:
    void slotForeverChanged(int checked);
};

#endif // APPOINTMENTDIALOG_H
