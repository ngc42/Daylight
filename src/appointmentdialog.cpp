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


void AppointmentDialog::userWantsNewAppointment()
{
    m_userWantsNewAppointment = true;
    m_appointment = new Appointment();
    m_appointment->generateUid();
    m_appointment->m_appRecurrence = nullptr;
    m_appointment->m_haveAlarm = false;
    m_appointment->m_haveRecurrence = false;
    m_appointment->m_appBasics = new AppointmentBasics();
    m_appointment->m_appBasics->m_uid = m_appointment->m_uid;
    m_appointment->m_appBasics->m_sequence = 0;     // start with a fresh sequence
    setUserCalendarIndexById( 0 );
}


void AppointmentDialog::userWantsModifyAppointment( const Appointment* apmData )
{
    // Dialog Management
    m_userWantsNewAppointment = false;
    m_storedOrigAppointment = apmData;

    // Appointment
    m_appointment = new Appointment();
    m_appointment->m_userCalendarId = apmData->m_userCalendarId;
    m_appointment->m_uid = apmData->m_uid;
    m_appointment->m_haveRecurrence = apmData->m_haveRecurrence;
    m_appointment->m_haveAlarm = apmData->m_haveAlarm;

    // AppointmentBasic
    m_appointment->m_appBasics = new AppointmentBasics( *apmData->m_appBasics );

    // AppointmentRecurrence
    if( m_appointment->m_haveRecurrence )
    {
        m_appointment->m_appRecurrence = new AppointmentRecurrence();
        m_appointment->m_appRecurrence->getAPartialCopy( *apmData->m_appRecurrence );
    }


    // gui part

    // Basic page
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
    if( not apmData->m_haveRecurrence )
        m_ui->basic_repeattype_combo->setCurrentIndex( NO_RECURRENCE );
    else
    {
        switch( apmData->m_appRecurrence->m_frequency )
        {
            case AppointmentRecurrence::RFT_SIMPLE_YEARLY:
            case AppointmentRecurrence::RFT_YEARLY:
                m_ui->basic_repeattype_combo->setCurrentIndex( YEARLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_MONTHLY:
            case AppointmentRecurrence::RFT_MONTHLY:
                m_ui->basic_repeattype_combo->setCurrentIndex( MONTHLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_WEEKLY:
            case AppointmentRecurrence::RFT_WEEKLY:
                m_ui->basic_repeattype_combo->setCurrentIndex( WEEKLY );
            break;
            case AppointmentRecurrence::RFT_SIMPLE_DAILY:
            case AppointmentRecurrence::RFT_DAILY:
                m_ui->basic_repeattype_combo->setCurrentIndex( DAILY );
            break;
            default:
                qDebug() << "ERR: unimplemented Selector in AppointmentDialog::setAppointmentValues()";
                Q_ASSERT( false );
            break;
        }
    }
}


void AppointmentDialog::increaseSequence()
{
    m_appointment->m_appBasics->m_sequence++;
}


void AppointmentDialog::setTimezoneIndexesByIanaId( const QByteArray iana1, const QByteArray iana2 )
{
    int index_iana1 = m_ui->basic_tz_start_combo->findText( iana1 );
    m_ui->basic_tz_start_combo->setCurrentIndex( index_iana1 );
    int index_iana2 = m_ui->basic_tz_end_combo->findText( iana2 );
    m_ui->basic_tz_end_combo->setCurrentIndex( index_iana2 );
}


void AppointmentDialog::collectAppointmentDataFromBasicPage()
{
    bool ok = false;
    // @fixme: missing timezones
    m_appointment->m_appBasics->m_dtStart = m_ui->basic_datetime_startInterval->dateTime();
    m_appointment->m_appBasics->m_dtEnd = m_ui->basic_datetime_endInterval->dateTime();
    m_appointment->m_appBasics->m_summary = m_ui->basic_title_le->text();
    m_appointment->m_appBasics->m_description = m_ui->basic_description->toPlainText();
    m_appointment->m_appBasics->m_busyFree = m_ui->basic_busy_check->isChecked() ?
                AppointmentBasics::BUSY : AppointmentBasics::FREE;
    m_appointment->m_userCalendarId = m_ui->basic_select_calendar_combo->currentData().toInt( &ok );

    // recurrence interval
    if( m_appointment->m_haveRecurrence )
    {
        bool complex_recurrence = m_appointment->m_appRecurrence->m_byMonthSet.count() > 0;
        complex_recurrence = complex_recurrence and m_appointment->m_appRecurrence->m_byWeekNumberSet.count() > 0;
        complex_recurrence = complex_recurrence and m_appointment->m_appRecurrence->m_byYearDaySet.count() > 0;
        complex_recurrence = complex_recurrence and m_appointment->m_appRecurrence->m_byMonthDaySet.count() > 0;
        complex_recurrence = complex_recurrence and m_appointment->m_appRecurrence->m_byDaySet.size() > 0;
        complex_recurrence = complex_recurrence and m_appointment->m_appRecurrence->m_bySetPosSet.count() > 0;

        switch( m_appointment->m_appRecurrence->m_frequency )
        {
            case AppointmentRecurrence::RFT_SIMPLE_YEARLY:
            case AppointmentRecurrence::RFT_YEARLY:
                m_appointment->m_appRecurrence->m_frequency =
                        complex_recurrence ? AppointmentRecurrence::RFT_YEARLY :
                                             AppointmentRecurrence::RFT_SIMPLE_YEARLY;
            break;
            case AppointmentRecurrence::RFT_SIMPLE_MONTHLY:
            case AppointmentRecurrence::RFT_MONTHLY:
                m_appointment->m_appRecurrence->m_frequency =
                        complex_recurrence ? AppointmentRecurrence::RFT_MONTHLY :
                                             AppointmentRecurrence::RFT_SIMPLE_MONTHLY;
            break;
            case AppointmentRecurrence::RFT_SIMPLE_WEEKLY:
            case AppointmentRecurrence::RFT_WEEKLY:
                m_appointment->m_appRecurrence->m_frequency =
                        complex_recurrence ? AppointmentRecurrence::RFT_WEEKLY :
                                             AppointmentRecurrence::RFT_SIMPLE_WEEKLY;
            break;
            case AppointmentRecurrence::RFT_SIMPLE_DAILY:
            case AppointmentRecurrence::RFT_DAILY:
                m_appointment->m_appRecurrence->m_frequency =
                        complex_recurrence ? AppointmentRecurrence::RFT_DAILY :
                                             AppointmentRecurrence::RFT_SIMPLE_DAILY;
            break;
            default:
                qDebug() << "ERR: unimplemented Selector in AppointmentDialog::collectAppointmentDataFromRecurrencePage()";
                Q_ASSERT( false );
        }
    }
}


void AppointmentDialog::collectAppointmentDataFromRecurrencePage()
{
    if( not m_appointment->m_haveRecurrence )
        return;

    /* Most of the tabs are handled by individual slots for their
     * Widgets:
     *  ByMonth-Tab
     *  ByWeekNo-Tab
     *  ByYearDay-Tab
     *  ByMonthDay-Tab
     *  ByDay-Tab
     */

    // Misc-Tab:

    m_appointment->m_appRecurrence->m_interval = m_ui->rec_misc_repeat_intervalnumber->value();

    switch( static_cast<RepeatRestrictionType>(m_repeatRestrictionButtonGroup->checkedId()) )
    {
        case REPEAT_FOREVER:
            m_appointment->m_appRecurrence->m_count = 0;            // forever
            m_appointment->m_appRecurrence->m_until = DateTime();   // invalid
        break;
        case REPEAT_COUNT:
            m_appointment->m_appRecurrence->m_count = m_ui->rec_misc_repeat_countnumber->value();
            m_appointment->m_appRecurrence->m_until = DateTime();   // invalid
        break;
        case REPEAT_UNTIL:
            m_appointment->m_appRecurrence->m_count = 0;
            m_appointment->m_appRecurrence->m_until = m_ui->rec_misc_repeat_untildate->dateTime();
        break;
    }

    bool ok = true;
    m_appointment->m_appRecurrence->m_startWeekday =
             static_cast<AppointmentRecurrence::WeekDay>(m_ui->rec_misc_weekstartday->currentData().toInt(&ok));
}


bool AppointmentDialog::modified() const
{
    // a new appointment is always modified
    if( m_userWantsNewAppointment ) return true;

    // check some appointment data
    if( not m_storedOrigAppointment->isPartiallyEqual( *m_appointment) ) return true;

    // check appointment basic data
    if( not ( *m_storedOrigAppointment->m_appBasics == *m_appointment->m_appBasics ) ) return true;

    // check some recurrence data
    if( m_storedOrigAppointment->m_haveRecurrence )
    {
        if( not m_storedOrigAppointment->m_appRecurrence->isPartiallyEqual( *m_appointment->m_appRecurrence ) )
            return true;
    }

    // @fixme: Alarm missing

    return false;
}


void  AppointmentDialog::deleteAppointment()
{
    if( m_appointment->m_haveAlarm )
    {
        while( not m_appointment->m_appAlarms.isEmpty() )
            delete m_appointment->m_appAlarms.first();
    }
    if( m_appointment->m_haveRecurrence )
        delete m_appointment->m_appRecurrence;
    delete m_appointment->m_appBasics;
    delete m_appointment;
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

    m_ui->tabWidget->setCurrentIndex( 0 );
}


void AppointmentDialog::reset( const QDate date )
{
    reset();
    setDefaultBasicInterval( date );
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


AppointmentDialog::RecurrenceFrequencyType AppointmentDialog::recurrence() const
{
    bool ok;
    int repeat = m_ui->basic_repeattype_combo->currentData().toInt(&ok);
    return static_cast<RecurrenceFrequencyType>(repeat);
}


AppointmentDialog::RepeatRestrictionType AppointmentDialog::repeatRestriction() const
{
    if( m_ui->rec_misc_repeat_count->isChecked() )
        return REPEAT_COUNT;
    if( m_ui->rec_misc_repeat_until->isChecked() )
        return REPEAT_UNTIL;
    return REPEAT_FOREVER;
}


void AppointmentDialog::resetRecurrencePage()
{
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
            if( not m_appointment->m_haveRecurrence )
            {
                m_appointment->m_haveRecurrence = true;
                m_appointment->m_appRecurrence = new AppointmentRecurrence();
            }
            m_appointment->m_appRecurrence->m_frequency = AppointmentRecurrence::RFT_YEARLY;
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
            if( not m_appointment->m_haveRecurrence )
            {
                m_appointment->m_haveRecurrence = true;
                m_appointment->m_appRecurrence = new AppointmentRecurrence();
            }
            m_appointment->m_appRecurrence->m_frequency = AppointmentRecurrence::RFT_MONTHLY;
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
            if( not m_appointment->m_haveRecurrence )
            {
                m_appointment->m_haveRecurrence = true;
                m_appointment->m_appRecurrence = new AppointmentRecurrence();
            }
            m_appointment->m_appRecurrence->m_frequency = AppointmentRecurrence::RFT_WEEKLY;
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
            if( not m_appointment->m_haveRecurrence )
            {
                m_appointment->m_haveRecurrence = true;
                m_appointment->m_appRecurrence = new AppointmentRecurrence();
            }
            m_appointment->m_appRecurrence->m_frequency = AppointmentRecurrence::RFT_DAILY;
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
            if( m_appointment->m_haveRecurrence )
            {
                m_appointment->m_haveRecurrence = false;
                delete m_appointment->m_appRecurrence;
            }
    }
}


void AppointmentDialog::slotMonthClicked( int id, bool checked )
{
    Q_ASSERT( m_appointment->m_haveRecurrence );
    if( checked )
        m_appointment->m_appRecurrence->m_byMonthSet.insert( id );
    else
        m_appointment->m_appRecurrence->m_byMonthSet.remove( id );
}


void AppointmentDialog::slotAddWeekNoClicked()
{
    int weekNumber = m_ui->rec_byweekno_weeknumber->value();
    if( weekNumber == 0 )
        return; // "0" does not make sense
    if( m_appointment->m_appRecurrence->m_byWeekNumberSet.contains( weekNumber) )
        return; // already in the set
    m_appointment->m_appRecurrence->m_byWeekNumberSet.insert( weekNumber );
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
        m_appointment->m_appRecurrence->m_byWeekNumberSet.remove( weekNumber );
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
    if( m_appointment->m_appRecurrence->m_byYearDaySet.contains( dayNumber) )
        return;
    m_appointment->m_appRecurrence->m_byYearDaySet.insert( dayNumber );
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
        m_appointment->m_appRecurrence->m_byYearDaySet.remove( dayNumber );
        int itemRow = m_ui->rec_byyearday_list->row( item );
        delete m_ui->rec_byyearday_list->takeItem( itemRow );
    }
}


void AppointmentDialog::slotAddMonthDayClicked()
{
    int dayNumber = m_ui->rec_bymonthday_daynumber->value();
    if( dayNumber == 0 )
        return;
    if( m_appointment->m_appRecurrence->m_byMonthDaySet.contains( dayNumber) )
        return;
    m_appointment->m_appRecurrence->m_byMonthDaySet.insert( dayNumber );
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
        m_appointment->m_appRecurrence->m_byMonthDaySet.remove( dayNumber );
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
    if( m_appointment->m_appRecurrence->m_byDaySet.find( dayElem ) != m_appointment->m_appRecurrence->m_byDaySet.end() )
        return;
    m_appointment->m_appRecurrence->m_byDaySet.insert( dayElem );
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
        // funny c++: we really have to make a pair, just to get rid of the set element:
        m_appointment->m_appRecurrence->m_byDaySet.erase(
                    std::make_pair( static_cast<AppointmentRecurrence::WeekDay>(dayElem.first), dayElem.second ) );
        int itemRow = m_ui->rec_byday_list->row( item );
        delete m_ui->rec_byday_list->takeItem( itemRow );
        qDebug() << m_appointment->m_appRecurrence->m_byDaySet.size();
    }
}


void AppointmentDialog::slotRepeatRestrictionRadiobuttonClicked(int id)
{
    // data is collected in collectAppointmentDataFromRecurrencePage().
    // Do not modify appointment here!
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
    if( m_appointment->m_appRecurrence->m_bySetPosSet.contains( posNumber) )
        return;
    m_appointment->m_appRecurrence->m_bySetPosSet.insert( posNumber );
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
        m_appointment->m_appRecurrence->m_bySetPosSet.remove( posNumber );
        int itemRow = m_ui->rec_misc_setpos_list->row( item );
        delete m_ui->rec_misc_setpos_list->takeItem( itemRow );
    }
}
