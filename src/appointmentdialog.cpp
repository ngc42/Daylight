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
#ifdef YYY
    m_ui->comboRepeat->addItem("No Recurrence", RecurrenceType::R_NO_RECURRENCE);
    m_ui->comboRepeat->addItem("every year", RecurrenceType::R_YEAR);
    m_ui->comboRepeat->addItem("every month", RecurrenceType::R_MONTH);
    m_ui->comboRepeat->addItem("every week", RecurrenceType::R_WEEK);
    m_ui->comboRepeat->addItem("every day", RecurrenceType::R_DAY);
#endif
    reset();
    connect(m_ui->checkWholeDay, SIGNAL(stateChanged(int)), this, SLOT(slotWholeDayChanged(int)));
    connect(m_ui->checkForever, SIGNAL(stateChanged(int)), this, SLOT(slotForeverChanged(int)));
}


AppointmentDialog::~AppointmentDialog()
{
    delete m_ui;
}


void AppointmentDialog::reset()
{
    // apointment
    m_ui->checkWholeDay->setChecked(false);
    m_ui->edReason->setText("new appointment");
    m_dtSaveStart = QDateTime::currentDateTime();
    m_dtSaveEnd = m_dtSaveStart.addSecs(3600);  // one hour
    m_ui->dateTimeStartInterval->setDateTime(m_dtSaveStart);
    m_ui->dateTimeEndInterval->setDateTime(m_dtSaveEnd);
    m_ui->comboSelectCalendar->setCurrentIndex(0);
    m_ui->comboRepeat->setCurrentIndex(0);
    m_ui->checkForever->setChecked(false);
    m_ui->dateTimeRepeatUntil->setDateTime(m_dtSaveEnd.addYears(3));
    m_ui->comboReminder->setCurrentIndex(0);
}


void AppointmentDialog::setFromAndTo(const QDate & date_from)
{
    QTime now = QTime::currentTime();
    m_dtSaveStart = QDateTime(date_from, now);
    m_dtSaveEnd = m_dtSaveStart.addSecs(3600);  // one hour
    m_ui->dateTimeStartInterval->setDateTime(m_dtSaveStart);
    m_ui->dateTimeEndInterval->setDateTime(m_dtSaveEnd);
    m_ui->dateTimeRepeatUntil->setDateTime(m_dtSaveEnd.addYears(3));
}


void AppointmentDialog::setUserCalendarInfos(QList<UserCalendarInfo*> & uciList)
{
    m_ui->comboSelectCalendar->clear();
    for(UserCalendarInfo* uci : uciList)
    {
        QPixmap pix(32, 32);
        pix.fill(uci->m_color);
        m_ui->comboSelectCalendar->addItem(pix, uci->m_title, uci->m_id);
    }
}


void AppointmentDialog::setAppointmentValues(Appointment* apmData)
{
    m_appointment = apmData;
#ifdef XXX
    // appointment
    m_ui->checkWholeDay->setChecked(apmData.m_allDay);
    m_ui->edReason->setText(apmData.m_title);
    m_dtSaveStart = apmData.m_startDt;
    m_dtSaveEnd = apmData.m_endDt;
    m_ui->dateTimeStartInterval->setDateTime(apmData.m_startDt);
    m_ui->dateTimeEndInterval->setDateTime(apmData.m_endDt);
    int index = m_ui->comboSelectCalendar->findData(apmData.m_userCalendarId);
    m_ui->comboSelectCalendar->setCurrentIndex(index);

    // recurrence
    index = m_ui->comboRepeat->findData(recData.m_type);
    m_ui->comboRepeat->setCurrentIndex(index);
    m_ui->checkForever->setChecked(recData.m_forever);
    m_ui->dateTimeRepeatUntil->setDateTime(recData.m_lastDt);
#endif
}


void AppointmentDialog::slotWholeDayChanged(int)
{
    if(m_ui->checkWholeDay->isChecked())
    {
        m_ui->dateTimeStartInterval->setDisplayFormat("dd.MM.yyyy");
        m_ui->dateTimeEndInterval->setDisplayFormat("dd.MM.yyyy");
    }
    else
    {
        m_ui->dateTimeStartInterval->setDisplayFormat("dd.MM.yyyy HH:mm");
        m_ui->dateTimeEndInterval->setDisplayFormat("dd.MM.yyyy HH:mm");
        m_ui->dateTimeStartInterval->setDateTime(m_dtSaveStart);
        m_ui->dateTimeEndInterval->setDateTime(m_dtSaveEnd);
    }
}


void AppointmentDialog::slotForeverChanged(int)
{
    if(m_ui->checkForever->isChecked())
    {
        m_ui->dateTimeRepeatUntil->setDisabled(true);
    }
    else
    {
        m_ui->dateTimeRepeatUntil->setEnabled(true);
    }
}
