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
#include "appointmentdialog.h"
#include "ui_appointmentdialog.h"

#include <QVariant>
#include <QDebug>

AppointmentDialog::AppointmentDialog(QWidget* parent) :
    QDialog(parent), m_ui(new Ui::AppointmentDialog)
{
    m_ui->setupUi(this);
    reset();
}


AppointmentDialog::~AppointmentDialog()
{
    delete m_ui;
}


void AppointmentDialog::reset()
{
    // Page Basic
    m_ui->basic_title_le->setText("new appointment");
    m_ui->basic_select_calendar_combo->setCurrentIndex(0);
    setDefaultBasicInterval( QDateTime::currentDateTime() );

    // Page Recurrence
    // ...
    // Page Alarm
    // ...
}


void AppointmentDialog::reset(const QDate date )
{
    reset();
    setDefaultBasicInterval( date );
}


void AppointmentDialog::setDefaultBasicInterval(const QDate date )
{
    QDateTime dt = QDateTime::currentDateTime();
    dt.setDate( date );
    setDefaultBasicInterval( dt );
}


void AppointmentDialog::setDefaultBasicInterval(const QDateTime dateTime )
{
    m_dtSaveStart = dateTime;
    m_dtSaveEnd = m_dtSaveStart.addSecs(3600);  // one hour
    m_ui->basic_datetime_startInterval->setDateTime(m_dtSaveStart);
    m_ui->basic_datetime_endInterval->setDateTime(m_dtSaveEnd);
}


void AppointmentDialog::setUserCalendarInfos(QList<UserCalendarInfo*> & uciList)
{
    m_ui->basic_select_calendar_combo->clear();
    for(UserCalendarInfo* uci : uciList)
    {
        QPixmap pix(32, 32);
        pix.fill(uci->m_color);
        m_ui->basic_select_calendar_combo->addItem(pix, uci->m_title, uci->m_id);
    }
}


void AppointmentDialog::setAppointmentValues(Appointment* apmData)
{
    m_appointment = apmData;
}


void AppointmentDialog::slotForeverChanged(int)
{

}
