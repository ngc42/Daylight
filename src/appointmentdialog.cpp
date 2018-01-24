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

    // signals basic tab
    connect( m_ui->basic_repeattype_combo, SIGNAL(currentIndexChanged(int)),
             this, SLOT(slotIndexChangedRecurrenceFrequency(int)) );
    // signals recurrence tab
    connect( m_ui->rec_byweekno_add, SIGNAL(clicked()), this, SLOT(slotAddWeekNoClicked()) );
    connect( m_ui->rec_byweekno_remove, SIGNAL(clicked()), this, SLOT(slotRemoveWeekNoClicked()) );

    connect( m_ui->rec_byyearday_add, SIGNAL(clicked()), this, SLOT(slotAddYearDayClicked()) );
    connect( m_ui->rec_byyearday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveYearDayClicked()) );

    connect( m_ui->rec_bymonthday_add, SIGNAL(clicked()), this, SLOT(slotAddMonthDayClicked()) );
    connect( m_ui->rec_bymonthday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveMonthDayClicked()) );

    connect( m_ui->rec_byday_add, SIGNAL(clicked()), this, SLOT(slotAddDayDayClicked()) );
    connect( m_ui->rec_byday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveDayDayClicked()) );

    connect( m_ui->rec_misc_setpos_add, SIGNAL(clicked()), this, SLOT(slotAddSetposClicked()) );
    connect( m_ui->rec_misc_setpos_remove, SIGNAL(clicked()), this, SLOT(slotRemoveSetposClicked()) );
}


AppointmentDialog::~AppointmentDialog()
{
    delete m_ui;
}


AppointmentDialog::RecurrenceFrequencyType AppointmentDialog::recurrence() const
{
    bool ok;
    int repeat = m_ui->basic_repeattype_combo->currentData().toInt(&ok);
    return static_cast<RecurrenceFrequencyType>(repeat);
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
    // these tabs and pages are enabled/disabled according to
    // RFC 5545, 3.3.10 (table).
    m_ui->tab_recurrence->setEnabled( true );
    switch( index )
    {
        case YEARLY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( true );
            m_ui->rec_page_byyearday->setEnabled( true );
            m_ui->rec_page_bymonthday->setEnabled( true );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( true );
            m_ui->rec_page_misc->setEnabled( true );
        break;
        case MONTHLY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( true );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( true );
            m_ui->rec_page_misc->setEnabled( true );
        break;
        case WEEKLY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( false );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( false );
            m_ui->rec_page_misc->setEnabled( true );
        break;
        case DAILY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( true );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( false );
            m_ui->rec_page_misc->setEnabled( true );
        break;
        default:
            m_ui->rec_page_bymonth->setDisabled( true );
            m_ui->rec_page_byweekno->setDisabled( true );
            m_ui->rec_page_byyearday->setDisabled( true );
            m_ui->rec_page_bymonthday->setDisabled( true );
            m_ui->rec_page_byday->setDisabled( true );
            m_ui->rec_page_misc->setDisabled( true );
            m_ui->tab_recurrence->setEnabled( true );   // disable tab
    }
}


void AppointmentDialog::slotAddWeekNoClicked()
{
    int weekNumber = m_ui->rec_byweekno_weeknumber->value();
    if( weekNumber == 0 )
        return; // "0" does not make sense
    if( m_weeksByWeekNo.contains( weekNumber) )
        return; // already in the set
    m_weeksByWeekNo.insert( weekNumber );
    QListWidgetItem* item = new QListWidgetItem( QString( "Week %1").arg( weekNumber) );
    item->setData( Qt::UserRole, weekNumber );
    m_ui->rec_byweekno_list->addItem( item );
}


void AppointmentDialog::slotRemoveWeekNoClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_byweekno_list->selectedItems();
    if( itemList.count() == 0 )
        return;     // no items selected
    bool ok;
    for( const QListWidgetItem* item : itemList )
    {
        int weekNumber = item->data( Qt::UserRole ).toInt(&ok);
        // remove from set
        m_weeksByWeekNo.remove( weekNumber );
        // remove the item from QListWidget
        int itemRow = m_ui->rec_byweekno_list->row( item );
        delete m_ui->rec_byweekno_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddYearDayClicked()
{
    int dayNumber = m_ui->rec_byyearday_daynumber->value();
    if( dayNumber == 0 )
        return;
    if( m_daysByYearDay.contains( dayNumber) )
        return;
    m_daysByYearDay.insert( dayNumber );
    QListWidgetItem* item = new QListWidgetItem( QString( "Day in Year %1").arg( dayNumber) );
    item->setData( Qt::UserRole, dayNumber );
    m_ui->rec_byyearday_list->addItem( item );
}


void AppointmentDialog::slotRemoveYearDayClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_byyearday_list->selectedItems();
    if( itemList.count() == 0 )
        return;
    bool ok;
    for( const QListWidgetItem* item : itemList )
    {
        int dayNumber = item->data( Qt::UserRole ).toInt(&ok);
        m_daysByYearDay.remove( dayNumber );
        int itemRow = m_ui->rec_byyearday_list->row( item );
        delete m_ui->rec_byyearday_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddMonthDayClicked()
{
    int dayNumber = m_ui->rec_bymonthday_daynumber->value();
    if( dayNumber == 0 )
        return;
    if( m_daysByMonthDay.contains( dayNumber) )
        return;
    m_daysByMonthDay.insert( dayNumber );
    QListWidgetItem* item = new QListWidgetItem( QString( "Day in Month %1").arg( dayNumber) );
    item->setData( Qt::UserRole, dayNumber );
    m_ui->rec_bymonthday_list->addItem( item );
}


void AppointmentDialog::slotRemoveMonthDayClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_bymonthday_list->selectedItems();
    if( itemList.count() == 0 )
        return;
    bool ok;
    for( const QListWidgetItem* item : itemList )
    {
        int dayNumber = item->data( Qt::UserRole ).toInt(&ok);
        m_daysByMonthDay.remove( dayNumber );
        int itemRow = m_ui->rec_bymonthday_list->row( item );
        delete m_ui->rec_bymonthday_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddDayDayClicked()
{
    bool ok;
    QString weekDayName = m_ui->rec_byday_weekdayname->currentText();
    int weekDay = m_ui->rec_byday_weekdayname->currentData( Qt::UserRole ).toInt(&ok);
    int dayNumber;
    RecurrenceFrequencyType recurrenceType = recurrence();
    if( recurrenceType == WEEKLY or recurrenceType == DAILY )
        dayNumber = 0;
    else
    {
        dayNumber = m_ui->rec_byday_daynumber->value();
        if( dayNumber == 0 )
            return;
    }

    // numeric value to store
    int data = weekDay * 1000 + 500 + dayNumber;

    if( m_weekDaysDaysByDay.contains( data ) )
        return;
    m_weekDaysDaysByDay.insert( data );
    QString text = dayNumber == 0 ? weekDayName : QString( "%1. %2" ).arg(dayNumber).arg(weekDayName);
    QListWidgetItem* item = new QListWidgetItem( text );
    item->setData( Qt::UserRole, data );
    m_ui->rec_byday_list->addItem( item );
}


void AppointmentDialog::slotRemoveDayDayClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_byday_list->selectedItems();
    if( itemList.count() == 0 )
        return;
    bool ok;
    for( const QListWidgetItem* item : itemList )
    {
        int data = item->data( Qt::UserRole ).toInt(&ok);
        m_weekDaysDaysByDay.remove( data );
        int itemRow = m_ui->rec_byday_list->row( item );
        delete m_ui->rec_byday_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddSetposClicked()
{
    int posNumber = m_ui->rec_misc_setpos_posnumber->value();
    if( posNumber == 0 )
        return;
    if( m_setPos.contains( posNumber) )
        return;
    m_setPos.insert( posNumber );
    QListWidgetItem* item = new QListWidgetItem( QString( "Setpos %1").arg( posNumber) );
    item->setData( Qt::UserRole, posNumber );
    m_ui->rec_misc_setpos_list->addItem( item );
}


void AppointmentDialog::slotRemoveSetposClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_misc_setpos_list->selectedItems();
    if( itemList.count() == 0 )
        return;
    bool ok;
    for( const QListWidgetItem* item : itemList )
    {
        int posNumber = item->data( Qt::UserRole ).toInt(&ok);
        m_setPos.remove( posNumber );
        int itemRow = m_ui->rec_misc_setpos_list->row( item );
        delete m_ui->rec_misc_setpos_list->takeItem( itemRow );
    }
}
