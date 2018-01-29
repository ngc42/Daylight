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
#include "appointmentdialog.h"
#include "ui_appointmentdialog.h"

#include <QDebug>
#include <QTimeZone>


AppointmentDialog::AppointmentDialog( QWidget* parent ) :
    QDialog( parent ),
    m_ui( new Ui::AppointmentDialog )
{
    m_ui->setupUi(this);

    m_repeatByMonthButtonGroup = new QButtonGroup( m_ui->rec_page_bymonth );
    m_repeatByMonthButtonGroup->setExclusive( false );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_jan, 1 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_feb, 2 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_mar, 3 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_apr, 4 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_may, 5 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_jun, 6 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_jul, 7 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_aug, 8 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_sep, 9 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_oct, 10 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_nov, 11 );
    m_repeatByMonthButtonGroup->addButton( m_ui->rec_bymonth_dec, 12 );


    m_repeatRestrictionButtonGroup = new QButtonGroup( m_ui->rec_misc_repeat_groupbox );
    m_repeatRestrictionButtonGroup->addButton( m_ui->rec_misc_repeat_forever, REPEAT_FOREVER );
    m_repeatRestrictionButtonGroup->addButton( m_ui->rec_misc_repeat_count, REPEAT_COUNT );
    m_repeatRestrictionButtonGroup->addButton( m_ui->rec_misc_repeat_until, REPEAT_UNTIL );

    m_ui->basic_repeattype_combo->addItem( "no recurrence", NO_RECURRENCE );
    m_ui->basic_repeattype_combo->addItem( "yearly", YEARLY );
    m_ui->basic_repeattype_combo->addItem( "monthly", MONTHLY );
    m_ui->basic_repeattype_combo->addItem( "weekly", WEEKLY );
    m_ui->basic_repeattype_combo->addItem( "daily", DAILY );


    // it starts on monday and iterates over all weekdays
    QDate d(2018, 1, 1);
    for(int i = 1; i < 8; i++ )
    {
        m_ui->rec_byday_weekdayname->addItem( d.toString( "dddd" ), static_cast<AppointmentRecurrence::WeekDay>(i) );
        m_ui->rec_misc_weekstartday->addItem( d.toString( "dddd" ), static_cast<AppointmentRecurrence::WeekDay>(i) );
        d = d.addDays( 1 );
    }

    setUpTimezones();

    // signals basic tab
    connect( m_ui->basic_repeattype_combo, SIGNAL(currentIndexChanged(int)),
             this, SLOT(slotIndexChangedRecurrenceFrequency(int)) );
    // signals recurrence tab
    connect( m_repeatByMonthButtonGroup, SIGNAL(buttonToggled(int,bool)),
             this, SLOT(slotMonthClicked(int, bool)) );

    connect( m_ui->rec_byweekno_add, SIGNAL(clicked()), this, SLOT(slotAddWeekNoClicked()) );
    connect( m_ui->rec_byweekno_remove, SIGNAL(clicked()), this, SLOT(slotRemoveWeekNoClicked()) );

    connect( m_ui->rec_byyearday_add, SIGNAL(clicked()), this, SLOT(slotAddYearDayClicked()) );
    connect( m_ui->rec_byyearday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveYearDayClicked()) );

    connect( m_ui->rec_bymonthday_add, SIGNAL(clicked()), this, SLOT(slotAddMonthDayClicked()) );
    connect( m_ui->rec_bymonthday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveMonthDayClicked()) );

    connect( m_ui->rec_byday_add, SIGNAL(clicked()), this, SLOT(slotAddDayDayClicked()) );
    connect( m_ui->rec_byday_remove, SIGNAL(clicked()), this, SLOT(slotRemoveDayDayClicked()) );

    connect( m_repeatRestrictionButtonGroup, SIGNAL(buttonClicked(int)),
             this, SLOT(slotRepeatRestrictionRadiobuttonClicked(int)) );

    connect( m_ui->rec_misc_setpos_add, SIGNAL(clicked()), this, SLOT(slotAddSetposClicked()) );
    connect( m_ui->rec_misc_setpos_remove, SIGNAL(clicked()), this, SLOT(slotRemoveSetposClicked()) );

    reset();
}


AppointmentDialog::~AppointmentDialog()
{
    delete m_repeatRestrictionButtonGroup;
    delete m_ui;
}


AppointmentDialog::RecurrenceFrequencyType AppointmentDialog::recurrence() const
{
    bool ok;
    int repeat = m_ui->basic_repeattype_combo->currentData().toInt(&ok);
    return static_cast<RecurrenceFrequencyType>(repeat);
}


bool AppointmentDialog::modified() const
{
    if( m_isNewAppointment )
        return false;
    // find out, if this appointment was modified by the user.
    bool ok;
    bool same = true;
    same = same and ( m_storedOrigAppointment->m_userCalendarId ==
                      m_ui->basic_select_calendar_combo->currentData().toInt( &ok ) );

    if( not same )  return true;
    // check appointment basic
    same = same and ( m_storedOrigAppointment->m_appBasics->m_dtStart == m_ui->basic_datetime_startInterval->dateTime() );
    same = same and ( m_storedOrigAppointment->m_appBasics->m_dtStart.timeZone().id() ==
            m_ui->basic_tz_start_combo->currentText() );
    same = same and ( m_storedOrigAppointment->m_appBasics->m_dtEnd == m_ui->basic_datetime_endInterval->dateTime() );
    same = same and ( m_storedOrigAppointment->m_appBasics->m_dtEnd.timeZone().id() ==
            m_ui->basic_tz_end_combo->currentText() );
    same = same and ( m_storedOrigAppointment->m_appBasics->m_summary == m_ui->basic_title_le->text() );
    same = same and ( m_storedOrigAppointment->m_appBasics->m_description == m_ui->basic_description->toPlainText() );
    same = same and ( (m_storedOrigAppointment->m_appBasics->m_busyFree == AppointmentBasics::BUSY ) ==
                      m_ui->basic_busy_check->isChecked() );

    if( not same )  return true;
    // check recurrence
    RecurrenceFrequencyType currentRecurrence = recurrence();

    if( m_storedOrigAppointment->m_haveRecurrence )
    {
        switch( m_storedOrigAppointment->m_appRecurrence->m_frequency )
        {
            case AppointmentRecurrence::RFT_SIMPLE_YEARLY:
            case AppointmentRecurrence::RFT_YEARLY:
                same = same and ( currentRecurrence == YEARLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_MONTHLY:
            case AppointmentRecurrence::RFT_MONTHLY:
                same = same and ( currentRecurrence == MONTHLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_WEEKLY:
            case AppointmentRecurrence::RFT_WEEKLY:
                same = same and ( currentRecurrence == WEEKLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_DAILY:
            case AppointmentRecurrence::RFT_DAILY:
                same = same and ( currentRecurrence == DAILY );
            break;
            default:
                same = false;
        }

        if( m_storedOrigAppointment->m_appRecurrence->m_haveCount )
        {
            same = same and
                   repeatRestriction() == REPEAT_COUNT and
                   m_storedOrigAppointment->m_appRecurrence->m_count == m_ui->rec_misc_repeat_countnumber->value();
        }
        else
        {
            if( m_storedOrigAppointment->m_appRecurrence->m_haveUntil )
                same = same and
                       repeatRestriction() == REPEAT_UNTIL and
                       m_storedOrigAppointment->m_appRecurrence->m_until == m_ui->rec_misc_repeat_untildate->dateTime();
            else
                same = same and repeatRestriction() == REPEAT_FOREVER;
        }

        same = same and (
               ( m_storedOrigAppointment->m_appRecurrence->m_haveInterval and
                 m_storedOrigAppointment->m_appRecurrence->m_interval == m_ui->rec_misc_repeat_intervalnumber->value() ) or
               ( not m_storedOrigAppointment->m_appRecurrence->m_haveInterval and
                m_ui->rec_misc_repeat_intervalnumber->value() == 1 ) );
        same = same and static_cast<int>(m_storedOrigAppointment->m_appRecurrence->m_startWeekday) ==
               m_ui->rec_misc_weekstartday->currentData().toInt(&ok);
        same = same and m_storedOrigAppointment->m_appRecurrence->m_byMonthSet == m_monthsMonthNo;
        same = same and m_storedOrigAppointment->m_appRecurrence->m_byWeekNumberSet == m_weeksByWeekNo;
        same = same and m_storedOrigAppointment->m_appRecurrence->m_byYearDaySet == m_daysByYearDay;
        same = same and m_storedOrigAppointment->m_appRecurrence->m_byMonthDaySet == m_daysByMonthDay;

        // @fixme: m_byDayMap missing

        same = same and m_storedOrigAppointment->m_appRecurrence->m_bySetPosSet == m_setPos;
    }
    else
    {
        same = same and currentRecurrence == NO_RECURRENCE;
    }

    // @fixme: Alarm missing

    return not same;
}


void AppointmentDialog::setUpTimezones()
{
    QTimeZone systemTz = QTimeZone::systemTimeZone();

    m_ui->basic_tz_start_combo->insertItem( 0, systemTz.id() );
    m_ui->basic_tz_end_combo->insertItem( 0, systemTz.id() );

    m_ui->basic_tz_start_combo->insertItem( 1, "UTC" );
    m_ui->basic_tz_end_combo->insertItem( 1, "UTC" );

    for( QByteArray tz : QTimeZone::availableTimeZoneIds() )
    {
        m_ui->basic_tz_start_combo->addItem( tz );
        m_ui->basic_tz_end_combo->addItem( tz);
    }
}


void AppointmentDialog::reset()
{
    // Page Basic
    m_ui->basic_title_le->setText( "new appointment" );
    m_ui->basic_select_calendar_combo->setCurrentIndex( 0 );
    setDefaultBasicInterval( QDateTime::currentDateTime() );
    m_ui->basic_tz_start_combo->setCurrentIndex( 0 );
    m_ui->basic_tz_end_combo->setCurrentIndex( 0 );
    m_ui->basic_busy_check->setChecked( false );
    m_ui->basic_description->clear();
    m_ui->basic_repeattype_combo->setCurrentIndex( 0 );

    // Page Recurrence
    resetRecurrencePage();

    // Page Alarm
    // ...

    // Disable Pages Recurrence and Alarm
    m_ui->tab_recurrence->setDisabled( true );
    m_ui->tab_alarm->setDisabled( true );
}


void AppointmentDialog::reset( const QDate date )
{
    reset();
    setDefaultBasicInterval( date );
}


void AppointmentDialog::resetRecurrencePage()
{
    m_monthsMonthNo.clear();
    m_ui->rec_bymonth_jan->setChecked( false );
    m_ui->rec_bymonth_feb->setChecked( false );
    m_ui->rec_bymonth_mar->setChecked( false );
    m_ui->rec_bymonth_apr->setChecked( false );
    m_ui->rec_bymonth_may->setChecked( false );
    m_ui->rec_bymonth_jun->setChecked( false );
    m_ui->rec_bymonth_jul->setChecked( false );
    m_ui->rec_bymonth_aug->setChecked( false );
    m_ui->rec_bymonth_sep->setChecked( false );
    m_ui->rec_bymonth_oct->setChecked( false );
    m_ui->rec_bymonth_nov->setChecked( false );
    m_ui->rec_bymonth_dec->setChecked( false );

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
    m_ui->rec_misc_repeat_intervalnumber->setValue( 1 );
    m_ui->rec_misc_repeat_intervalnumber->setSuffix( "" );
    m_ui->rec_misc_repeat_forever->click();
    m_ui->rec_misc_repeat_untildate->setDateTime( m_dtSaveEnd.addYears( 10 ) );
    m_ui->rec_misc_repeat_countnumber->setValue( 10 );

    m_ui->rec_misc_setpos_posnumber->setValue( 1 );
    m_ui->rec_misc_setpos_list->clear();
}


void AppointmentDialog::setDefaultBasicInterval( const QDate date )
{
    QDateTime dt = QDateTime::currentDateTime();
    dt.setDate( date );
    setDefaultBasicInterval( dt );
}


void AppointmentDialog::setDefaultBasicInterval( const QDateTime dateTime )
{
    m_dtSaveStart = dateTime;
    m_dtSaveEnd = m_dtSaveStart.addSecs( 3600 );  // one hour
    m_ui->basic_datetime_startInterval->setDateTime( m_dtSaveStart );
    m_ui->basic_datetime_endInterval->setDateTime( m_dtSaveEnd );
}


void AppointmentDialog::setUserCalendarInfos( QList<UserCalendarInfo*> & uciList )
{
    m_ui->basic_select_calendar_combo->clear();
    for( UserCalendarInfo* uci : uciList )
    {
        QPixmap pix(32, 32);
        pix.fill( uci->m_color );
        m_ui->basic_select_calendar_combo->addItem( pix, uci->m_title, uci->m_id );
    }
}


void AppointmentDialog::setUserCalendarIndexById( const int usercalendarId )
{
    int index = m_ui->basic_select_calendar_combo->findData( usercalendarId );
    if( index == -1 and usercalendarId != 0 )
    {
        index = m_ui->basic_select_calendar_combo->findData( 0 );
    }
    m_ui->basic_select_calendar_combo->setCurrentIndex( index );
}


void AppointmentDialog::createNewAppointment()
{
    m_isNewAppointment = true;
    m_appointment = new Appointment();
    m_appointment->generateUid();
    m_sequence = 0;     // start with a fresh sequence
    setUserCalendarIndexById( 0 );
}


void AppointmentDialog::setAppointmentValues( const Appointment* apmData )
{
    m_isNewAppointment = false;
    m_storedOrigAppointment = apmData;
    m_appointment = new Appointment();
    m_appointment->m_uid = apmData->m_uid;
    m_appointment->m_userCalendarId = apmData->m_userCalendarId;
    m_sequence = apmData->m_appBasics->m_sequence;

    // gui part
    m_ui->basic_title_le->setText( apmData->m_appBasics->m_summary );
    setUserCalendarIndexById( apmData->m_userCalendarId );
    m_ui->basic_datetime_startInterval->setDateTime( apmData->m_appBasics->m_dtStart );
    m_ui->basic_datetime_endInterval->setDateTime( apmData->m_appBasics->m_dtEnd );
    setTimezoneIndexesByIanaId(
                apmData->m_appBasics->m_dtStart.timeZone().id(),
                apmData->m_appBasics->m_dtEnd.timeZone().id() );
    m_ui->basic_busy_check->setChecked(
                apmData->m_appBasics->m_busyFree == AppointmentBasics::BUSY );
    m_ui->basic_description->setPlainText( apmData->m_appBasics->m_description );
}


void AppointmentDialog::setTimezoneIndexesByIanaId( const QByteArray iana1, const QByteArray iana2 )
{
    int index_iana1 = m_ui->basic_tz_start_combo->findText( iana1 );
    m_ui->basic_tz_start_combo->setCurrentIndex( index_iana1 );
    int index_iana2 = m_ui->basic_tz_end_combo->findText( iana2 );
    m_ui->basic_tz_end_combo->setCurrentIndex( index_iana2 );
}


void AppointmentDialog::collectAppointmentData()
{
    bool ok = false;
    AppointmentBasics* basics = new AppointmentBasics();
    basics->m_uid = m_appointment->m_uid;
    basics->m_sequence = m_sequence;
    // @fixme: missing timezones
    basics->m_dtStart = m_ui->basic_datetime_startInterval->dateTime();
    basics->m_dtEnd = m_ui->basic_datetime_endInterval->dateTime();
    basics->m_summary = m_ui->basic_title_le->text();
    basics->m_description = m_ui->basic_description->toPlainText();
    basics->m_busyFree = m_ui->basic_busy_check->isChecked() ?
                AppointmentBasics::BUSY : AppointmentBasics::FREE;
    m_appointment->m_appBasics = basics;
    m_appointment->m_userCalendarId = m_ui->basic_select_calendar_combo->currentData().toInt( &ok );
    m_appointment->m_appRecurrence = nullptr;
    m_appointment->m_haveAlarm = false;
    m_appointment->m_haveRecurrence = false;
}


AppointmentDialog::RepeatRestrictionType AppointmentDialog::repeatRestriction() const
{
    if( m_ui->rec_misc_repeat_count->isChecked() )
        return REPEAT_COUNT;
    if( m_ui->rec_misc_repeat_until->isChecked() )
        return REPEAT_UNTIL;
    return REPEAT_FOREVER;
}


void AppointmentDialog::slotIndexChangedRecurrenceFrequency( int index )
{
    // these tabs and pages are enabled/disabled according to
    // RFC 5545, 3.3.10 (table).
    resetRecurrencePage();
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
            m_ui->rec_misc_repeat_intervalnumber->setSuffix( " years" );
        break;
        case MONTHLY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( true );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( true );
            m_ui->rec_page_misc->setEnabled( true );
            m_ui->rec_misc_repeat_intervalnumber->setSuffix( " months" );
        break;
        case WEEKLY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( false );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( false );
            m_ui->rec_page_misc->setEnabled( true );
            m_ui->rec_misc_repeat_intervalnumber->setSuffix( " weeks" );
        break;
        case DAILY:
            m_ui->rec_page_bymonth->setEnabled( true );
            m_ui->rec_page_byweekno->setEnabled( false );
            m_ui->rec_page_byyearday->setEnabled( false );
            m_ui->rec_page_bymonthday->setEnabled( true );
            m_ui->rec_page_byday->setEnabled( true );
            m_ui->rec_byday_daynumber->setEnabled( false );
            m_ui->rec_page_misc->setEnabled( true );
            m_ui->rec_misc_repeat_intervalnumber->setSuffix( " days" );
        break;
        default:
            m_ui->rec_page_bymonth->setDisabled( true );
            m_ui->rec_page_byweekno->setDisabled( true );
            m_ui->rec_page_byyearday->setDisabled( true );
            m_ui->rec_page_bymonthday->setDisabled( true );
            m_ui->rec_page_byday->setDisabled( true );
            m_ui->rec_page_misc->setDisabled( true );
            m_ui->tab_recurrence->setEnabled( true );   // disable tab
            m_ui->rec_misc_repeat_intervalnumber->setSuffix( "" );
    }
}


void AppointmentDialog::slotMonthClicked( int id, bool checked )
{
    if( checked )
        m_monthsMonthNo.insert( id );
    else
        m_monthsMonthNo.remove( id );
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
        int weekNumber = item->data( Qt::UserRole ).toInt( &ok );
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
    QListWidgetItem* item = new QListWidgetItem( QString( "Day in Year %1" ).arg( dayNumber) );
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
    QListWidgetItem* item = new QListWidgetItem( QString( "Day in Month %1" ).arg( dayNumber) );
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
        int dayNumber = item->data( Qt::UserRole ).toInt( &ok );
        m_daysByMonthDay.remove( dayNumber );
        int itemRow = m_ui->rec_bymonthday_list->row( item );
        delete m_ui->rec_bymonthday_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddDayDayClicked()
{
    bool ok;
    QString weekDayName = m_ui->rec_byday_weekdayname->currentText();
    int weekDay = m_ui->rec_byday_weekdayname->currentData( Qt::UserRole ).toInt( &ok );
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

    std::pair<AppointmentRecurrence::WeekDay, int> dayElem =
            std::make_pair( static_cast<AppointmentRecurrence::WeekDay>(weekDay), dayNumber );
    if( m_weekDaysDaysByDay.find( dayElem ) != m_weekDaysDaysByDay.end() )
        return;
    m_weekDaysDaysByDay.insert( dayElem );
    QString text = dayNumber == 0 ? weekDayName : QString( "%1. %2" ).arg( dayNumber ).arg( weekDayName );
    QListWidgetItem* item = new QListWidgetItem( text );
    item->setData( Qt::UserRole, QVariant::fromValue( std::make_pair(weekDay, dayNumber) ) );
    m_ui->rec_byday_list->addItem( item );
}


void AppointmentDialog::slotRemoveDayDayClicked()
{
    QList<QListWidgetItem*> itemList = m_ui->rec_byday_list->selectedItems();
    if( itemList.count() == 0 )
        return;
    for( const QListWidgetItem* item : itemList )
    {
        std::pair<int,int> dayElem  = item->data( Qt::UserRole ).value<std::pair<int,int>>();
        m_weekDaysDaysByDay.erase(
                    std::make_pair( static_cast<AppointmentRecurrence::WeekDay>(dayElem.first), dayElem.second ) );
        int itemRow = m_ui->rec_byday_list->row( item );
        delete m_ui->rec_byday_list->takeItem( itemRow );
        qDebug() << m_weekDaysDaysByDay.size();
    }
}


void AppointmentDialog::slotRepeatRestrictionRadiobuttonClicked(int id)
{
    switch ( static_cast<RepeatRestrictionType>(id) )
    {
        case REPEAT_FOREVER:
            m_ui->rec_misc_repeat_countnumber->setDisabled( true );
            m_ui->rec_misc_repeat_untildate->setDisabled( true );
        break;
        case REPEAT_COUNT:
            m_ui->rec_misc_repeat_countnumber->setEnabled( true );
            m_ui->rec_misc_repeat_untildate->setDisabled( true );
        break;
        case REPEAT_UNTIL:
            m_ui->rec_misc_repeat_countnumber->setDisabled( true );
            m_ui->rec_misc_repeat_untildate->setEnabled( true );
        break;
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
        int posNumber = item->data( Qt::UserRole ).toInt( &ok );
        m_setPos.remove( posNumber );
        int itemRow = m_ui->rec_misc_setpos_list->row( item );
        delete m_ui->rec_misc_setpos_list->takeItem( itemRow );
    }
}
