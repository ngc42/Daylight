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
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingsdialog.h"
#include <QDebug>

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_storage = new Storage(this);
    m_settingsManager = new SettingsManager();
    m_ui->statusBar->hide();

    // toolbar items
    m_toolbarDateLabel = new QToolButton();
    QFont f = m_toolbarDateLabel->font();
    f.setBold(true);
    m_toolbarDateLabel->setFont(f);
    m_toolbarDateLabel->setDefaultAction(m_ui->actionShowHideNavigationDlg);
    m_toolbarDateLabel->setText("Hello, Welt!");
    m_ui->mainToolBar->insertWidget(m_ui->actionNextDate, m_toolbarDateLabel);

    m_toolbarUserCalendarMenu = new QToolButton();
    m_toolbarUserCalendarMenu->setDefaultAction(m_ui->actionAddUserCalendar);
    m_toolbarUserCalendarMenu->setPopupMode(QToolButton::MenuButtonPopup);
    m_ui->mainToolBar->insertWidget(m_ui->actionShowYear, m_toolbarUserCalendarMenu);
    m_ui->mainToolBar->insertSeparator(m_ui->actionShowYear);

    // ATM, there is only one view active at a time.
    m_groupCalendarAppearance = new QActionGroup(this);
    m_groupCalendarAppearance->addAction(m_ui->actionShowYear);
    m_groupCalendarAppearance->addAction(m_ui->actionShowMonth);
    m_groupCalendarAppearance->addAction(m_ui->actionShow3Weeks);
    m_groupCalendarAppearance->addAction(m_ui->actionShowWeek);
    m_groupCalendarAppearance->addAction(m_ui->actionShowDay);
    m_groupCalendarAppearance->setExclusive(true);

    // user calendar pool
    m_userCalendarPool = new UserCalendarPool(this);

    // this is where the calendars are shown
    m_scene = new CalendarScene(m_settingsManager->currentSettings(), this);
    m_ui->graphicsView->setScene(m_scene);

    // a storage for all our appointments, romantic dates and dentist visits
    m_appointmentPool = new AppointmentPool();
    // dialog to set up the appointments, non-modal
    m_appointmentDialog = new AppointmentDialog();
    m_appointmentDialog->hide();

    // Navigation dialog
    m_navigationDialog = new NavigationDialog(this);
    m_navigationDialog->hide();

    // new user calendar
    m_userCalendarNewDialog = new UserCalendarNew(this);
    m_userCalendarNewDialog->hide();

    // connect main signals
    connect(m_ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));
    connect(m_ui->actionPreferences, SIGNAL(triggered()), this, SLOT(slotSettingsDialog()));

    // connect view signals
    connect(m_ui->actionShowYear, SIGNAL(triggered()), this, SLOT(slotShowYear()));
    connect(m_ui->actionShowMonth, SIGNAL(triggered()), this, SLOT(slotShowMonth()));
    connect(m_ui->actionShow3Weeks, SIGNAL(triggered()), this, SLOT(slotShow3Weeks()));
    connect(m_ui->actionShowWeek, SIGNAL(triggered()), this, SLOT(slotShowWeek()));
    connect(m_ui->actionShowDay, SIGNAL(triggered()), this, SLOT(slotShowDay()));

    // connect calendar navigation
    connect(m_ui->actionShowHideNavigationDlg, SIGNAL(triggered()), this, SLOT(slotShowHideNavigationDlg()));
    connect(m_navigationDialog, SIGNAL(signalCurrentPageChanged(int,int)), this, SLOT(slotSetDate(int, int)));
    connect(m_navigationDialog, SIGNAL(signalClicked(QDate)), this, SLOT(slotSetDate(QDate)));
    connect(m_ui->actionGotoToday, SIGNAL(triggered()), this, SLOT(slotSetToday()));
    connect(m_ui->actionPrevDate, SIGNAL(triggered()), this, SLOT(slotSetPreviousDate()));
    connect(m_ui->actionNextDate, SIGNAL(triggered()), this, SLOT(slotSetNextDate()));

    // appointments
    connect(m_navigationDialog, SIGNAL(signalActivated(QDate)), this, SLOT(slotAppointmentDlgStart(QDate)));
    connect(m_scene, SIGNAL(signalDateClicked(QDate)), this, SLOT(slotAppointmentDlgStart(QDate)));
    connect(m_appointmentDialog, SIGNAL(finished(int)), this, SLOT(slotAppointmentDlgFinished(int)));
    connect(m_ui->actionAddAppointment, SIGNAL(triggered()), this, SLOT(slotAppointmentDlgStart()));
    connect(m_scene, SIGNAL(signalReconfigureAppointment(int)), this, SLOT(slotReconfigureAppointment(int)));
    connect(m_scene, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)), Qt::QueuedConnection);

    // user calendars
    connect(m_ui->actionAddUserCalendar, SIGNAL(triggered()), this, SLOT(slotAddUserCalendar()));
    connect(m_userCalendarPool, SIGNAL(signalUserCalendarInUseModified()), this, SLOT(slotUserCalendarInUseModified()));
    connect(m_userCalendarNewDialog, SIGNAL(finished(int)), this, SLOT(slotAddUserCalendarDlgFinished(int)));
    connect(m_ui->actionCalendarManager, SIGNAL(triggered()), this, SLOT(slotCalendarManagerDialog()));

    // storage load data
    connect(m_storage, SIGNAL(signalLoadedUserCalendarFromStorage(UserCalendarInfo* &)),
            m_userCalendarPool, SLOT(slotAddUserCalendarFromStorage(UserCalendarInfo* &)));
    m_storage->loadUserCalendarInfo();
    disconnect(m_storage, SIGNAL(signalLoadedUserCalendarFromStorage(UserCalendarInfo* &)),
            m_userCalendarPool, SLOT(slotAddUserCalendarFromStorage(UserCalendarInfo* &)));
    connect(m_storage, SIGNAL(signalLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)),
            this, SLOT(slotLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)));
    m_storage->loadAppointmentData();
    disconnect(m_storage, SIGNAL(signalLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)),
               this, SLOT(slotLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)));

    QMenu* tmp = m_userCalendarPool->calendarMenu();
    m_toolbarUserCalendarMenu->setMenu(tmp);
    connect(m_userCalendarPool, SIGNAL(signalUserCalendarDataModified(int,QColor,QString,bool)),
            m_storage, SLOT(slotUserCalendarDataModified(int,QColor,QString,bool)));
    // - end storage


    // what to show depends on config
    switch( m_settingsManager->startView() )
    {
        case SettingStartWithView::START_YEAR:
            emit m_ui->actionShowYear->trigger();
            break;
        case SettingStartWithView::START_MONTH:
            emit m_ui->actionShowMonth->trigger();
            break;
        case SettingStartWithView::START_3WEEKS:
            emit m_ui->actionShow3Weeks->trigger();
            break;
        case SettingStartWithView::START_WEEK:
            emit m_ui->actionShowWeek->trigger();
            break;
        case SettingStartWithView::START_DAY:
            emit m_ui->actionShowDay->trigger();
            break;
        default:
            emit m_ui->actionShowYear->trigger();
            break;
    }

    slotSetDate( m_settingsManager->startDate() );
}


MainWindow::~MainWindow()
{
    delete m_ui;
}


/* collects all resizes which affect CalendarScene and sends them to
   m_scene. CalendarScene cares for their own geometry.
   fixme: if user wants to resize, there are MANY! resize events and every
   geometry update takes a lot of time.
*/
void MainWindow::resizeCalendarView()
{
    m_scene->updateSize(m_ui->graphicsView->size());
}


void MainWindow::showAppointments(const QDate & date)
{
    int weekStartDay = m_settingsManager->weekStartDay();
    QList<UserCalendarInfo*> showHideItems = m_userCalendarPool->calendarInfos();
    QList<Appointment*> listYear = m_appointmentPool->appointmentForYear(date);
    QList<Appointment*> listMonth = m_appointmentPool->appointmentForMonth(date, weekStartDay);
    QList<Appointment*> list3Weeks = m_appointmentPool->appointmentFor3Weeks(date, weekStartDay);
    QList<Appointment*> listWeek = m_appointmentPool->appointmentForWeek(date, weekStartDay);
    QList<Appointment*> listDay = m_appointmentPool->appointmentForDay(date);

    // remove all items from listYear, listMonth, ... for which the calendar is disabled
    for(const UserCalendarInfo* uci : showHideItems)
    {
        if(! uci->m_isVisible)
        {
            for(Appointment* apm : listYear)
            {
                if(apm->m_appointmentData.m_userCalendarId == uci->m_id)
                    listYear.removeOne(apm);
            }
            for(Appointment* apm : listMonth)
            {
                if(apm->m_appointmentData.m_userCalendarId == uci->m_id)
                    listMonth.removeOne(apm);
            }
            for(Appointment* apm : list3Weeks)
            {
                if(apm->m_appointmentData.m_userCalendarId == uci->m_id)
                    list3Weeks.removeOne(apm);
            }
            for(Appointment* apm : listWeek)
            {
                if(apm->m_appointmentData.m_userCalendarId == uci->m_id)
                    listWeek.removeOne(apm);
            }
            for(Appointment* apm : listDay)
            {
                if(apm->m_appointmentData.m_userCalendarId == uci->m_id)
                    listDay.removeOne(apm);
            }
        }
    }

    // show the rest
    m_scene->setAppointmentsForYear(listYear);
    m_scene->setAppointmentsForMonth(listMonth);
    m_scene->setAppointmentsFor3Weeks(list3Weeks);
    m_scene->setAppointmentsForWeek(listWeek);
    m_scene->setAppointmentsForDay(listDay);
}


void MainWindow::resizeEvent(QResizeEvent* )
{
    resizeCalendarView();
}


void MainWindow::moveEvent(QMoveEvent* event)
{
    QSize tbSize = m_toolbarDateLabel->size();
    QPoint pos = mapToGlobal(m_ui->mainToolBar->pos()) + m_toolbarDateLabel->pos() + QPoint(10, tbSize.height());
    if(m_navigationDialog->isVisible())
    {
        m_navigationDialog->move(pos);
    }
    QMainWindow::moveEvent(event);
}


/* Start Settingsdialog and wait till finished */
void MainWindow::slotSettingsDialog()
{
    SettingsData settings = m_settingsManager->currentSettings();
    SettingsDialog* dlg = new SettingsDialog(settings, this);
    if( dlg->exec() == QDialog::Accepted )
    {
        if(dlg->dataModified())
        {
            m_settingsManager->setSettings(dlg->settings());
            m_scene->setSettings(m_settingsManager->currentSettings());
            slotSetDate(m_scene->date());  // update appointments and toolbar view, needed if SettingsData::m_weekStartDay was modified
        }
    }
}


void MainWindow::slotCalendarManagerDialog()
{
    CalendarManagerDialog* dlg = new CalendarManagerDialog(m_userCalendarPool->calendarInfos(),
                                                           m_appointmentPool->appointmentData(),
                                                           this);
    connect(dlg, SIGNAL(signalDeleteCalendar(int)), this, SLOT(slotDeleteCalendar(int)));
    connect(dlg, SIGNAL(signalModifyCalendar(int,QString,QColor)), this, SLOT(slotModifyCalendar(int,QString,QColor)));
    connect(dlg, SIGNAL(signalModifyAppointmentCalendar(int,int)), this, SLOT(slotModifyAppointmentCalendar(int, int)));
    dlg->exec();
}


/* user clicks on the page buttons of the small calendar
 * check, if the date is valid and forward the date to slotSetDate(QDate).
 * An invalid date is most often just the 29. february in non-leap-years.
*/
void MainWindow::slotSetDate(int year, int month)
{
    QDate d(year, month, m_scene->date().day());
    if(!d.isValid())
    {
        d.setDate(year, month, 1);
        d.setDate(year, month, d.daysInMonth());
    }
    slotSetDate(d);
}


/* User clicks on a date in the small calendar or we receive a forward from
 * slotSetDate(int, int) from above.
 * in case of an invalid date, set the QDate::currentDate() for the calendar. */
void MainWindow::slotSetDate(const QDate & date)
{
    QDate dateTmp(date);
    
    if(! dateTmp.isValid())
    {
        dateTmp = QDate::currentDate();
    }
    m_scene->setDate(dateTmp);
    m_navigationDialog->slotSetSelectedDate(dateTmp);
    showAppointments(dateTmp);

    switch(m_scene->showView())
    {
        case CalendarShow::SHOW_UNKNOWN:
            break;
        case CalendarShow::SHOW_YEAR:
            m_toolbarDateLabel->setText(m_scene->date().toString("yyyy"));
            break;
        case CalendarShow::SHOW_MONTH:
            m_toolbarDateLabel->setText(m_scene->date().toString("MMMM yyyy"));
            break;
        case CalendarShow::SHOW_3WEEKS:
        {
            QDate start(date);
            int dayDelta = m_settingsManager->weekStartDay() - date.dayOfWeek();
            if (dayDelta > 0)
                dayDelta = dayDelta - 7;
            start = start.addDays( dayDelta ).addDays(-7);
            QDate end = start.addDays(20);
            QString s = QString("%1 - %2").arg(start.toString("dd.MM")).arg(end.toString("dd.MM.yyyy"));
            m_toolbarDateLabel->setText(s);
        }
            break;
        case CalendarShow::SHOW_WEEK:
        {
            QDate start(date);
            int dayDelta = m_settingsManager->weekStartDay() - date.dayOfWeek();
            if (dayDelta > 0)
                dayDelta = dayDelta - 7;
            start = start.addDays( dayDelta );
            QDate end = start.addDays(6);
            QString s = QString("Week %1, %2 - %3").arg(start.weekNumber()).arg(start.toString("dd.MM")).arg(end.toString("dd.MM.yyyy"));
            m_toolbarDateLabel->setText(s);
        }
            break;
        case CalendarShow::SHOW_DAY:
            m_toolbarDateLabel->setText(m_scene->date().toString("dd.MM.yyyy"));
            break;
    }
    m_settingsManager->setSelectedDate(dateTmp);
}



/* sets the date to current date.
 * menu action */
void MainWindow::slotSetToday()
{
    slotSetDate(QDate::currentDate());
}


/* sets the previous year, month, ... .
 * there is a setting controlling this action: "m_settingsManager->week3AddDays()"
 * toolbar action */
void MainWindow::slotSetPreviousDate()
{
    QDate actualDate = m_scene->date();
    QDate newDate;
    switch(m_scene->showView())
    {
        case CalendarShow::SHOW_UNKNOWN:
            newDate = actualDate;
            break;
        case CalendarShow::SHOW_YEAR:
            newDate = actualDate.addYears(-1);
            break;
        case CalendarShow::SHOW_MONTH:
            newDate = actualDate.addMonths(-1);
            break;
        case CalendarShow::SHOW_3WEEKS:
            newDate = actualDate.addDays(-m_settingsManager->week3AddDays());
            break;
        case CalendarShow::SHOW_WEEK:
            newDate = actualDate.addDays(-7);
            break;
        case CalendarShow::SHOW_DAY:
            newDate = actualDate.addDays(-1);
            break;
    }
    slotSetDate(newDate);
}


/* sets the next year, month, ... .
 * there is a setting controlling this action: "m_settingsManager->week3AddDays()"
 * toolbar action */
void MainWindow::slotSetNextDate()
{
    QDate actualDate = m_scene->date();
    QDate newDate;
    switch(m_scene->showView())
    {
        case CalendarShow::SHOW_UNKNOWN:
            newDate = actualDate;
            break;
        case CalendarShow::SHOW_YEAR:
            newDate = actualDate.addYears(1);
            break;
        case CalendarShow::SHOW_MONTH:
            newDate = actualDate.addMonths(1);
            break;
        case CalendarShow::SHOW_3WEEKS:
            newDate = actualDate.addDays(m_settingsManager->week3AddDays());
            break;
        case CalendarShow::SHOW_WEEK:
            newDate = actualDate.addDays(7);
            break;
        case CalendarShow::SHOW_DAY:
            newDate = actualDate.addDays(1);
            break;
    }
    slotSetDate(newDate);
}


/* Double Click or return pressed on a selected date in the small calendar or item in calendar view.
 * Start the AppointmentDialog and let the user create a new appointment.
 * - Note: The Appointmentdialog is non-modal, so it does not block. We just call
 * - show() and hide()
 * date parameter defaults to currentDate(), so this method is used as a menu slot too. */
void MainWindow::slotAppointmentDlgStart(const QDate & date)
{
    m_appointmentDialog->reset();
    m_appointmentDialog->setFromAndTo(date);
    QList<UserCalendarInfo*> uciList = m_userCalendarPool->calendarInfos();
    if(uciList.count() == 0)
        return;
    m_appointmentDialog->setUserCalendarInfos(uciList);
    m_appointmentDialog->show();
}


/* User wants to reconfigure an appointment. Take the data from this appointment
 * and show the appointmet dialog with the given data.
 * Communicate over appointment id with slotAppointmentDlgFinished(). */
void MainWindow::slotReconfigureAppointment(int appointmentId)
{
    QList<UserCalendarInfo*> uciList = m_userCalendarPool->calendarInfos();
    Appointment* apm = m_appointmentPool->appointmentById(appointmentId);
    m_appointmentDialog->setUserCalendarInfos(uciList);
    m_appointmentDialog->setAppointmentValues(apm->m_appointmentData, apm->m_recurrenceData);
    m_appointmentDialog->show();
}


/* Appointment dialog finishes, user has set up a new appointment or modified
 * an existing one. If user clicks on "OK", we add the appointment to the
 * database or change the existing item. */
void MainWindow::slotAppointmentDlgFinished(int returncode)
{
    if(QDialog::Accepted == returncode)
    {
        QColor calendarColor;
        AppointmentData apmData = m_appointmentDialog->appointmentData();
        // find user calendar color of this appointment
        for(UserCalendarInfo* uci : m_userCalendarPool->calendarInfos())
        {
            if(uci->m_id == apmData.m_userCalendarId)
            {
                calendarColor = uci->m_color;
                break;
            }
        }
        // change the appointment, if there was alredy set an appointment id
        if(m_appointmentDialog->appointmentId() == 0)
        {
            RecurrenceData recData = m_appointmentDialog->recurrenceData();
            int id = m_appointmentPool->add(apmData, recData, calendarColor);
            apmData.m_appointmentId = id;
            recData.m_appointmentId = id;
            m_storage->slotAppointmentAdd(apmData, recData);
            qDebug() << "AppointmentDlgFinished";
        }
        else
        {
            m_appointmentPool->modifyAppointment(apmData, m_appointmentDialog->recurrenceData(), calendarColor);
            m_storage->modifyAppointment(apmData, m_appointmentDialog->recurrenceData());
        }
        QDate date = m_scene->date();
        showAppointments(date);
    }
    m_appointmentDialog->hide();
}


/* Delete an appointmet by a given id and show the appointments again. */
void MainWindow::slotDeleteAppointment(int appointmentId)
{
    if( m_settingsManager->warnOnAppointmentDelete() )
    {
        QMessageBox::StandardButton button = QMessageBox::question(this, "Delete appointment", "Do you really want to delete this appointment?");
        if(button == QMessageBox::No)
            return;
    }
    m_appointmentPool->removeById(appointmentId);
    m_storage->removeAppointment(appointmentId);
    QDate date = m_scene->date();
    showAppointments(date);
}


void MainWindow::slotLoadedAppointmentFromStorage(const AppointmentData & apmData, const RecurrenceData & recData)
{
    QColor color = m_userCalendarPool->color(apmData.m_userCalendarId);
    m_appointmentPool->addAppointmentFromStorage(apmData, recData, color);
}


void MainWindow::slotShowHideNavigationDlg()
{
    if(m_navigationDialog->isHidden())
    {
        QSize tbSize = m_toolbarDateLabel->size();
        QPoint pos = mapToGlobal(m_ui->mainToolBar->pos()) + m_toolbarDateLabel->pos() + QPoint(10, tbSize.height());
        m_navigationDialog->move(pos);
        m_navigationDialog->show();
    }
    else
        m_navigationDialog->hide();
}


void MainWindow::slotShowYear()
{
    m_scene->slotShowYear();
    m_toolbarDateLabel->setText(m_scene->date().toString("yyyy"));
    m_settingsManager->setSelectedView(SettingStartWithView::START_YEAR);
}


void MainWindow::slotShowMonth()
{
    m_scene->slotShowMonth();
    m_toolbarDateLabel->setText(m_scene->date().toString("MMMM yyyy"));
    m_settingsManager->setSelectedView(SettingStartWithView::START_MONTH);
}


void MainWindow::slotShow3Weeks()
{
    m_scene->slotShow3Weeks();
    QDate start(m_scene->date());
    start = start.addDays( m_settingsManager->weekStartDay() - start.dayOfWeek() ).addDays(-7);
    QDate end = start.addDays(20);
    QString s = QString("%1 - %2").arg(start.toString("dd.MM")).arg(end.toString("dd.MM.yyyy"));
    m_toolbarDateLabel->setText(s);
    m_settingsManager->setSelectedView(SettingStartWithView::START_3WEEKS);
}


void MainWindow::slotShowWeek()
{
    m_scene->slotShowWeek();
    QDate start(m_scene->date());
    start = start.addDays( m_settingsManager->weekStartDay() - start.dayOfWeek() );
    QDate end = start.addDays(6);
    QString s = QString("Week %1, %2 - %3").arg(start.weekNumber()).arg(start.toString("dd.MM")).arg(end.toString("dd.MM.yyyy"));
    m_toolbarDateLabel->setText(s);
    m_settingsManager->setSelectedView(SettingStartWithView::START_WEEK);
}


void MainWindow::slotShowDay()
{
    m_scene->slotShowDay();
    m_toolbarDateLabel->setText(m_scene->date().toString("dd.MM.yyyy"));
    m_settingsManager->setSelectedView(SettingStartWithView::START_DAY);
}


void MainWindow::slotAddUserCalendar()
{
    m_userCalendarNewDialog->reset();
    m_userCalendarNewDialog->setUserCalendarInfos(m_userCalendarPool->calendarInfos());
    m_userCalendarNewDialog->show();
}


void MainWindow::slotAddUserCalendarDlgFinished(int returncode)
{
    if(QDialog::Accepted == returncode)
    {
        // fixme: title should not be empty
        const UserCalendarInfo* uci = m_userCalendarPool->addUserCalendar(
                    m_userCalendarNewDialog->calendarColor(),
                    m_userCalendarNewDialog->calendarTitle(),
                    true);
        m_storage->insertUserCalendarInfo(uci);

        QMenu* tmp = m_userCalendarPool->calendarMenu();
        m_toolbarUserCalendarMenu->setMenu(tmp);
    }
    m_userCalendarNewDialog->hide();
}


/* User selects or unselects a calendar in m_userCalendarPool.
 * We need to show/hide the calendars.
*/
void MainWindow::slotUserCalendarInUseModified()
{
    showAppointments(m_scene->date());
}


/* User wants to delete a calendar. Delete it in m_userCalendarPool and m_storage.
 * connected in MainWindow::slotCalendarManagerDialog() */
void MainWindow::slotDeleteCalendar(const int calendarId)
{
    m_appointmentPool->removeByCalendarId(calendarId);
    m_userCalendarPool->removeUserCalendar(calendarId);
    m_storage->removeUserCalendar(calendarId);
    showAppointments(m_scene->date());
}


/* User changes title and color, but not visibility.
 * connected in MainWindow::slotCalendarManagerDialog() */
void MainWindow::slotModifyCalendar(const int calendarId, const QString & title, const QColor & color)
{
    qDebug() << "MW:: slotmodifycalendar";
    m_appointmentPool->setColorForId(calendarId, color);
    bool visible = m_userCalendarPool->isVisible(calendarId);
    m_userCalendarPool->setData(calendarId, color, title, visible);
    m_storage->slotUserCalendarDataModified(calendarId, color, title, visible);
    showAppointments(m_scene->date());
}


/* appointment gets new calendar
 * connected in MainWindow::slotCalendarManagerDialog() */
void MainWindow::slotModifyAppointmentCalendar(const int appId, const int newCalendarId)
{
    QColor c = m_userCalendarPool->color(newCalendarId);
    m_appointmentPool->setAppointmentCalendarForId(appId, newCalendarId, c);
    m_storage->setAppointmentsCalendar(appId, newCalendarId);
    showAppointments(m_scene->date());
}
