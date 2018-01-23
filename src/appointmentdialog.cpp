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

    m_ui->basic_repeattype_combo->addItem( "no recurrence", NO_RECURRENCE );
    m_ui->basic_repeattype_combo->addItem( "yearly", YEARLY );
    m_ui->basic_repeattype_combo->addItem( "monthly", MONTHLY );
    m_ui->basic_repeattype_combo->addItem( "weekly", WEEKLY );
    m_ui->basic_repeattype_combo->addItem( "daily", DAILY );


    // it starts on monday and iterates over all weekdays
    QDate d(2018, 1, 1);
    for(int i = 1; i < 8; i++ )
    {
        m_ui->rec_byday_weekdayname->addItem( d.toString( "dddd" ), i );
        m_ui->rec_misc_weekstartday->addItem( d.toString( "dddd" ), i );
        d = d.addDays( 1 );
    }

    reset();

    connect( m_ui->basic_repeattype_combo, SIGNAL(currentIndexChanged(int)),
             this, SLOT(slotIndexChangedRecurrenceFrequency(int)) );
}


AppointmentDialog::~AppointmentDialog()
{
    delete m_ui;
}


void AppointmentDialog::reset()
{
    // Page Basic
    m_ui->basic_title_le->setText("new appointment");
    m_ui->basic_select_calendar_combo->setCurrentIndex( 0 );
    setDefaultBasicInterval( QDateTime::currentDateTime() );
    m_ui->basic_repeattype_combo->setCurrentIndex( 0 );

    // Page Recurrence
    m_ui->rec_bymonth_jan->setChecked(false);
    m_ui->rec_bymonth_feb->setChecked(false);
    m_ui->rec_bymonth_mar->setChecked(false);
    m_ui->rec_bymonth_apr->setChecked(false);
    m_ui->rec_bymonth_may->setChecked(false);
    m_ui->rec_bymonth_jun->setChecked(false);
    m_ui->rec_bymonth_jul->setChecked(false);
    m_ui->rec_bymonth_aug->setChecked(false);
    m_ui->rec_bymonth_sep->setChecked(false);
    m_ui->rec_bymonth_oct->setChecked(false);
    m_ui->rec_bymonth_nov->setChecked(false);
    m_ui->rec_bymonth_dec->setChecked(false);

    m_ui->rec_byweekno_weeknumber->setValue( 1 );
    m_ui->rec_byweekno_list->clear();

    m_ui->rec_byyearday_daynumber->setValue( 1 );
    m_ui->rec_byyearday_list->clear();

    m_ui->rec_bymonthday_daynumber->setValue( 1 );
    m_ui->rec_bymonthday_list->clear();

    m_ui->rec_byday_daynumber->setValue( 1 );
    m_ui->rec_byday_weekdayname->setCurrentIndex( 0 );
    m_ui->rec_byday_list->clear();

    m_ui->rec_misc_weekstartday->setCurrentIndex( 0 );
    m_ui->rec_misc_repeatuntil_forever->setChecked(true);
    m_ui->rec_misc_repeatuntil_datetime->setDateTime( m_dtSaveEnd.addYears( 10 ) );
    m_ui->rec_misc_setpos_posnumber->setValue( 1 );
    m_ui->rec_misc_setpos_list->clear();

    // Page Alarm
    // ...

    // Disable Pages Recurrence and Alarm
    m_ui->tab_recurrence->setDisabled( true );
    m_ui->tab_alarm->setDisabled( true );
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


void AppointmentDialog::slotIndexChangedRecurrenceFrequency( int index )
{
    qDebug() << index;
    switch( index )
    {
        case YEARLY:
        case MONTHLY:
        case WEEKLY:
        case DAILY:
        default:
            m_ui->rec_page_bymonth->setDisabled( true );
            m_ui->rec_page_byweekno->setDisabled( true );
            m_ui->rec_page_byyearday->setDisabled( true );
            m_ui->rec_page_bymonthday->setDisabled( true );
            m_ui->rec_page_byday->setDisabled( true );
            m_ui->rec_page_misc->setDisabled( true );
    }

}


void AppointmentDialog::slotForeverChanged(int)
{

}
