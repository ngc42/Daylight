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
#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QSettings>
#include <QDialog>
#include <QDate>



namespace Ui {
class SettingsDialog;
}



/* different types of views to show at program start. Show a year view, ..., or whatever was
 * shown the last time, the program was used. */
enum SettingStartWithView { START_YEAR, START_MONTH, START_3WEEKS, START_WEEK, START_DAY, START_LASTVIEW };



/* this is what we store in a config file. */
struct SettingsData
{
    // common
    SettingStartWithView m_startView;   // start with this view on next program start
    bool m_startWithToday;              // true: start with today, false: start with last selected date
    bool m_warnMeOnAppointmentDelete;   // true: show a dialog on appointment delete
    // day time
    int m_dayStartHour;                 // 0...20
    int m_dayEndHour;                   // m_dayStartHour + 1 ... 24
    // week config
    int m_weekStartDay;                 // 1: monday, 2: tuesday, ...
    int m_3weeks_add_value;             // 1 week... 3 weeks. affects the number of weeks using "previous" and "next" buttons in toolbar
    // last use
    int m_last_date_day;                // last selected date
    int m_last_date_month;
    int m_last_date_year;
    SettingStartWithView m_last_view;  // last selected view (except START_LASTVIEW) used for m_startView if START_LASTVIEW
};



/* the settings manager reads the config file at program start and stores the current
 *  config values during program execution. The informations are mostly used in MainWindow,
 *  weekStartDay is mainly used in CalendarScene. */
class SettingsManager : public QSettings
{
    Q_OBJECT

public:
    explicit SettingsManager(QObject* parent = 0);
    bool validateSettings(const SettingsData data) const;
    const SettingsData currentSettings() const { return m_settings; }
    void setSettings(const SettingsData data);
    void setSelectedDate(const QDate & date);
    void setSelectedView(const SettingStartWithView view);
    SettingStartWithView startView() const;
    const QDate startDate() const;
    int weekStartDay() const { return m_settings.m_weekStartDay; }
    int week3AddDays() const { return 7 * m_settings.m_3weeks_add_value; }
    bool warnOnAppointmentDelete() const { return m_settings.m_warnMeOnAppointmentDelete; }

private:
    void defaultSettings();
    void settingsToGroups();
    void groupsToSettings();
    SettingsData m_settings;

signals:
public slots:
};



/* a dialog to set the config variables. User can select the first day of the week,
 * for example. This dialog is fired in MainWindow as a result of a menu action.
 * This dialog has an UI file, settingsdialog.ui. */
class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(const SettingsData & data, QWidget *parent = 0);
    bool dataModified() const { return m_somethingModified; }
    const SettingsData settings() const { return m_settings; }
    ~SettingsDialog();

private:
    Ui::SettingsDialog *m_ui;
    SettingsData m_settings;
    bool m_somethingModified;

    void setupCommonPage(const SettingsData & data);
    void setupDayPage(const SettingsData & data);
    void setupWeekPage(const SettingsData & data);

signals:
public slots:
private slots:
    void slotCommonStartWithViewChanged(int index);
    void slotCommonStartWithTodayChanged(int state);    // check box
    void slotCommonWarningChanged(int state);           // check box
    void slotDayStartTimeChanged(int index);
    void slotDayEndTimeChanged(int index);
    void slotWeekStartDayChanged(int index);
    void slotWeekNextAddsChanged(int index);
};

#endif // SETTINGSDIALOG_H
