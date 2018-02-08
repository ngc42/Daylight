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
#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include <QDebug>



/***********************************************************
********** SettingsManager *********************************
***********************************************************/

SettingsManager::SettingsManager(QObject* parent) :
    QSettings(QSettings::IniFormat, QSettings::UserScope, "Free Software", "Daylight", parent)
{

    qDebug() << "read settings ";

    if(contains("COMMON/StartView"))        // if available
    {
        // ini file --> SettingsData
        groupsToSettings(); // read
        if(! validateSettings(m_settings))
        {
            defaultSettings();
            settingsToGroups();
            qDebug() << "Write default Settings";
        }
    }
    else
    {
        qDebug() << " -> default";
        defaultSettings();
        settingsToGroups(); // write
    }
}


bool SettingsManager::validateSettings(const SettingsData data) const
{
    bool vCommon = (data.m_startView >= SettingStartWithView::START_YEAR) and (data.m_startView <= SettingStartWithView::START_LASTVIEW);
    bool vDays = (data.m_dayStartHour >= 0) and (data.m_dayStartHour < 24) and (data.m_dayEndHour > 0) and (data.m_dayEndHour <= 24) and (data.m_dayStartHour < data.m_dayEndHour);
    bool vWeek = (data.m_weekStartDay > 0) and (data.m_weekStartDay <= 7) and (data.m_3weeks_add_value > 0) and (data.m_3weeks_add_value < 4);
    QDate d(data.m_last_date_year, data.m_last_date_month, data.m_last_date_day);
    bool vLast = (data.m_last_view >= SettingStartWithView::START_YEAR) and (data.m_last_view <= SettingStartWithView::START_LASTVIEW) and d.isValid();

    bool all = vCommon and vDays and vWeek and vLast;
    if(!all)
        qDebug() << "ERROR: cannot validate settings in SettingsManager::validateSettings()";
    return all;
}


/* new settings, possibly from dialog or somewhere else. Push them to the groups.
 * fixme: make sure, data makes sense */
void SettingsManager::setSettings( const SettingsData data )
{
    if( validateSettings( data ) )
    {
        m_settings = data;
        settingsToGroups();
    }
    else
        defaultSettings();
}


void SettingsManager::setSelectedDate( const QDate date )
{
    QDate d( date );
    if( not d.isValid() )
        d = QDate::currentDate();
    m_settings.m_last_date_day = d.day();
    m_settings.m_last_date_month = d.month();
    m_settings.m_last_date_year = d.year();
    settingsToGroups();
}


void SettingsManager::setSelectedView(const SettingStartWithView view)
{
    if(view >= SettingStartWithView::START_YEAR and view < SettingStartWithView::START_LASTVIEW)
        m_settings.m_last_view = view;
    else
        m_settings.m_last_view = SettingStartWithView::START_YEAR;
    settingsToGroups();
}


SettingStartWithView SettingsManager::startView() const
{
    if(m_settings.m_startView == SettingStartWithView::START_LASTVIEW)
        return m_settings.m_last_view;
    else
        return m_settings.m_startView;
}


const QDate SettingsManager::startDate() const
{
    if(! m_settings.m_startWithToday)
    {
        QDate d(m_settings.m_last_date_year, m_settings.m_last_date_month, m_settings.m_last_date_day);
        if(d.isValid())
            return d;
    }
    return QDate::currentDate();
}


void SettingsManager::defaultSettings()
{
    // common
    m_settings.m_startView = SettingStartWithView::START_YEAR;
    m_settings.m_startWithToday = true;
    m_settings.m_warnMeOnAppointmentDelete = false;
    // day
    m_settings.m_dayStartHour = 8;
    m_settings.m_dayEndHour = 20;
    // week
    m_settings.m_weekStartDay = 1;
    m_settings.m_3weeks_add_value = 1;
    // last used
    QDate d = QDate::currentDate();
    m_settings.m_last_date_day = d.day();
    m_settings.m_last_date_month = d.month();
    m_settings.m_last_date_year = d.year();
    m_settings.m_last_view = SettingStartWithView::START_YEAR;
}


void SettingsManager::settingsToGroups()
{
    beginGroup("COMMON");
    setValue("StartView", m_settings.m_startView);
    setValue("StartWithToday", m_settings.m_startWithToday);
    setValue("WarnOnAppointmentDelete", m_settings.m_warnMeOnAppointmentDelete);
    endGroup();
    beginGroup("DAYS");
    setValue("DayStartHour", m_settings.m_dayStartHour);
    setValue("DayEndHour", m_settings.m_dayEndHour);
    endGroup();
    beginGroup("WEEKS");
    setValue("WeekFirstDay", m_settings.m_weekStartDay);
    setValue("Week3AddValue", m_settings.m_3weeks_add_value);
    endGroup();
    beginGroup("LASTUSE");
    setValue("LastDateDay", m_settings.m_last_date_day);
    setValue("LastDateMonth", m_settings.m_last_date_month);
    setValue("LastDateYear", m_settings.m_last_date_year);
    setValue("LastView", m_settings.m_last_view);
    endGroup();
    sync();
}


/* Whatever gets read from ini file, we have to convert this to our SettingsData.
 * Make sure, that all items are defaultet to the default values, thats the reason we call "defaultSettings()" first.
 * Please do not just copy the values or leave default unset.
 * fixme: make sure, data makes sense
 * */
void SettingsManager::groupsToSettings()
{
    bool ok;
    defaultSettings();
    beginGroup("COMMON");
    m_settings.m_startView = SettingStartWithView( value("StartView", m_settings.m_startView).toInt(&ok));
    m_settings.m_startWithToday= value("StartWithToday", m_settings.m_startWithToday).toBool();
    m_settings.m_warnMeOnAppointmentDelete = value("WarnOnAppointmentDelete", m_settings.m_warnMeOnAppointmentDelete).toBool();
    endGroup();
    beginGroup("DAYS");
    m_settings.m_dayStartHour = value("DayStartHour", m_settings.m_dayStartHour).toInt(&ok);
    m_settings.m_dayEndHour = value("DayEndHour", m_settings.m_dayEndHour).toInt(&ok);
    endGroup();
    beginGroup("WEEKS");
    m_settings.m_weekStartDay = value("WeekFirstDay", m_settings.m_weekStartDay).toInt(&ok);
    m_settings.m_3weeks_add_value = value("Week3AddValue", m_settings.m_3weeks_add_value).toInt(&ok);
    endGroup();
    beginGroup("LASTUSE");
    m_settings.m_last_date_day = value("LastDateDay", m_settings.m_last_date_day).toInt(&ok);
    m_settings.m_last_date_month = value("LastDateMonth", m_settings.m_last_date_month).toInt(&ok);
    m_settings.m_last_date_year = value("LastDateYear", m_settings.m_last_date_year).toInt(&ok);
    m_settings.m_last_view = SettingStartWithView( value("LastView", m_settings.m_last_view).toInt(&ok) );
    endGroup();
}



/***********************************************************
********** SettingsDialog **********************************
***********************************************************/

SettingsDialog::SettingsDialog(const SettingsData & data, QWidget *parent) :
    QDialog(parent), m_ui(new Ui::SettingsDialog), m_settings(data), m_somethingModified(false)
{
    m_ui->setupUi(this);
    setupCommonPage(data);
    setupDayPage(data);
    setupWeekPage(data);
    m_ui->tabWidget->setCurrentIndex(0);
}


SettingsDialog::~SettingsDialog()
{
    delete m_ui;
}


void SettingsDialog::setupCommonPage(const SettingsData & data)
{
    m_ui->comboCommonStartWithView->addItem("Year", SettingStartWithView::START_YEAR);
    m_ui->comboCommonStartWithView->addItem("Month", SettingStartWithView::START_MONTH);
    m_ui->comboCommonStartWithView->addItem("3-Weeks", SettingStartWithView::START_3WEEKS);
    m_ui->comboCommonStartWithView->addItem("Week", SettingStartWithView::START_WEEK);
    m_ui->comboCommonStartWithView->addItem("Day", SettingStartWithView::START_DAY);
    m_ui->comboCommonStartWithView->addItem("Last selected view", SettingStartWithView::START_LASTVIEW);
    // set item by data
    int index = m_ui->comboCommonStartWithView->findData( data.m_startView );
    index = index == -1 ? 0 : index;
    m_ui->comboCommonStartWithView->setCurrentIndex(index);

    m_ui->cbCommonStartToday->setChecked(data.m_startWithToday);
    m_ui->cbCommonWarning->setChecked(data.m_warnMeOnAppointmentDelete);

    connect(m_ui->comboCommonStartWithView, SIGNAL(currentIndexChanged(int)), this, SLOT(slotCommonStartWithViewChanged(int)));
    connect(m_ui->cbCommonStartToday, SIGNAL(stateChanged(int)), this, SLOT(slotCommonStartWithTodayChanged(int)));
    connect(m_ui->cbCommonWarning, SIGNAL(stateChanged(int)), this, SLOT(slotCommonWarningChanged(int)));
}


void SettingsDialog::setupDayPage(const SettingsData & data)
{
    // populate start day time
    m_ui->comboDayStartTime->addItem("Midnight", 0);
    m_ui->comboDayStartTime->addItem("2", 2);
    m_ui->comboDayStartTime->addItem("4", 4);
    m_ui->comboDayStartTime->addItem("6", 6);
    m_ui->comboDayStartTime->addItem("8", 8);
    m_ui->comboDayStartTime->addItem("10", 10);

    // set item by data
    int index = m_ui->comboDayStartTime->findData(data.m_dayStartHour);
    index = index == -1 ? 0 : index;
    m_ui->comboDayStartTime->setCurrentIndex(index);

    // populate end day time
    m_ui->comboDayEndTime->addItem("Midnight", 24);
    m_ui->comboDayEndTime->addItem("12", 12);
    m_ui->comboDayEndTime->addItem("14", 14);
    m_ui->comboDayEndTime->addItem("16", 16);
    m_ui->comboDayEndTime->addItem("18", 18);
    m_ui->comboDayEndTime->addItem("20", 20);
    m_ui->comboDayEndTime->addItem("22", 22);

    // set current item by data
    index = m_ui->comboDayEndTime->findData(data.m_dayEndHour);
    index = index == -1 ? 0 : index;
    m_ui->comboDayEndTime->setCurrentIndex(index);

    connect(m_ui->comboDayStartTime, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDayStartTimeChanged(int)));
    connect(m_ui->comboDayEndTime, SIGNAL(currentIndexChanged(int)), this, SLOT(slotDayEndTimeChanged(int)));
}


void SettingsDialog::setupWeekPage(const SettingsData & data)
{
    m_ui->comboWeekStartDay->addItem("Monday", 1);
    m_ui->comboWeekStartDay->addItem("Tuesday", 2);
    m_ui->comboWeekStartDay->addItem("Wednesday", 3);
    m_ui->comboWeekStartDay->addItem("Thursday", 4);
    m_ui->comboWeekStartDay->addItem("Friday", 5);
    m_ui->comboWeekStartDay->addItem("Satueday", 6);
    m_ui->comboWeekStartDay->addItem("Sunday", 7);
    // set current item by given data
    int index = m_ui->comboWeekStartDay->findData(data.m_weekStartDay);
    index = index == -1 ? 0 : index;
    m_ui->comboWeekStartDay->setCurrentIndex(index);

    m_ui->comboWeekNextAdds->addItem("1 week", 1);
    m_ui->comboWeekNextAdds->addItem("2 weeks", 2);
    m_ui->comboWeekNextAdds->addItem("3 weeks", 3);

    index = m_ui->comboWeekNextAdds->findData(data.m_3weeks_add_value);
    index = index == -1 ? 0 : index;
    m_ui->comboWeekNextAdds->setCurrentIndex(index);

    connect(m_ui->comboWeekStartDay, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWeekStartDayChanged(int)));
    connect(m_ui->comboWeekNextAdds, SIGNAL(currentIndexChanged(int)), this, SLOT(slotWeekNextAddsChanged(int)));
}


void SettingsDialog::slotCommonStartWithViewChanged(int)
{
    m_somethingModified = true;
    bool ok;
    m_settings.m_startView = SettingStartWithView( m_ui->comboCommonStartWithView->currentData().toInt(&ok) );
}


void SettingsDialog::slotCommonStartWithTodayChanged(int)
{
    m_somethingModified = true;
    m_settings.m_startWithToday = m_ui->cbCommonStartToday->checkState() == Qt::Checked;
}


void SettingsDialog::slotCommonWarningChanged(int)
{
    m_somethingModified = true;
    m_settings.m_warnMeOnAppointmentDelete = m_ui->cbCommonWarning->checkState() == Qt::Checked;
}


void SettingsDialog::slotDayStartTimeChanged(int)
{
    m_somethingModified = true;
    bool ok;
    m_settings.m_dayStartHour = m_ui->comboDayStartTime->currentData().toInt(&ok);
}


void SettingsDialog::slotDayEndTimeChanged(int)
{
    m_somethingModified = true;
    bool ok;
    m_settings.m_dayEndHour = m_ui->comboDayEndTime->currentData().toInt(&ok);
}


void SettingsDialog::slotWeekStartDayChanged(int)
{
    m_somethingModified = true;
    bool ok;
    m_settings.m_weekStartDay = m_ui->comboWeekStartDay->currentData().toInt(&ok);
}


void SettingsDialog::slotWeekNextAddsChanged(int)
{
    m_somethingModified = true;
    bool ok;
    m_settings.m_3weeks_add_value = m_ui->comboWeekNextAdds->currentData().toInt(&ok);
}


