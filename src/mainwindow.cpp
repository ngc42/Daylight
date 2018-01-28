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
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>

#include "calendarmanagerdialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_settingsManager = new SettingsManager();
    m_storage = new Storage();
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

    // this is where the calendars are shown
    m_scene = new CalendarScene(m_settingsManager->currentSettings(), this);
    m_ui->graphicsView->setScene(m_scene);

    // in-app database
    m_eventPool = new EventPool();

    // Navigation dialog
    m_navigationDialog = new NavigationDialog(this);
    m_navigationDialog->hide();

    // dialog to set up the appointments, non-modal
    m_appointmentDialog = new AppointmentDialog();
    m_appointmentDialog->hide();

    // user calendar pool
    m_userCalendarPool = new UserCalendarPool(this);
    m_storage->loadUserCalendarInfo( m_userCalendarPool );

    // User calendars in Toolbar to switch on/off individual menus
    QMenu* userCalendarMenu = m_userCalendarPool->calendarMenu();
    m_toolbarUserCalendarMenu->setMenu( userCalendarMenu );

    // new user calendar
    m_userCalendarNewDialog = new UserCalendarNew(this);
    m_userCalendarNewDialog->hide();

    // ical import dialog
    m_icalImportDialog = new IcalImportDialog( this );
    m_icalImportDialog->hide();

    // connect main signals
    connect(m_ui->actionPreferences, SIGNAL(triggered()), this, SLOT(slotSettingsDialog()));
    connect(m_ui->actionOpenICalFile, SIGNAL(triggered()), this, SLOT(slotOpenIcalFile()));
    connect(m_ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

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
    connect(m_scene, SIGNAL(signalReconfigureAppointment(QString)), this, SLOT(slotReconfigureAppointment(QString)));
#ifdef XXX

    connect(m_scene, SIGNAL(signalDeleteAppointment(int)), this, SLOT(slotDeleteAppointment(int)), Qt::QueuedConnection);
#endif

    // user calendars
    connect(m_ui->actionAddUserCalendar, SIGNAL(triggered()), this, SLOT(slotAddUserCalendarDlg()));
    //connect(m_userCalendarPool, SIGNAL(signalUserCalendarInUseModified()), this, SLOT(slotUserCalendarInUseModified()));
    connect(m_userCalendarNewDialog, SIGNAL(finished(int)), this, SLOT(slotAddUserCalendarDlgFinished(int)));
    connect(m_ui->actionCalendarManager, SIGNAL(triggered()), this, SLOT(slotCalendarManagerDialog()));

    // ical import dialog
    connect(m_icalImportDialog, SIGNAL(sigFinishReadingFiles()),
            this, SLOT(slotImportFromFileFinished()) );

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
    delete m_scene;
    delete m_ui;
}


void MainWindow::showAppointments(const QDate &date)
{
    QVector<Appointment*> appointmentsThisYear;
    if( not m_eventPool->queryMarker( date.year()) )
    {
        m_storage->loadAppointmentByYear( date.year(), appointmentsThisYear );
        for(Appointment* &a :appointmentsThisYear )
        {
            int calId = a->m_userCalendarId;
            QColor calColor = m_userCalendarPool->color( calId );
            a->setEventColor( calColor );
            m_eventPool->addAppointment(a);
        }
        m_eventPool->addMarker( date.year() );
    }
    //int weekStartDay = m_settingsManager->weekStartDay();
    QList<UserCalendarInfo*> showHideItems = m_userCalendarPool->calendarInfos();
    QVector<Event> listYear = m_eventPool->eventsByYear( date.year() );

#if XXX
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
#endif
    // show the rest
    m_scene->setAppointmentsForYear( listYear );
#ifdef xxx
    m_scene->setAppointmentsForMonth(listMonth);
    m_scene->setAppointmentsFor3Weeks(list3Weeks);
    m_scene->setAppointmentsForWeek(listWeek);
    m_scene->setAppointmentsForDay(listDay);
#endif
}


void MainWindow::resizeEvent( QResizeEvent* )
{
    // @fixme: Why does graphicsView does not
    // auto-resize with parents?
    m_ui->graphicsView->resize( centralWidget()->size() );
    m_scene->updateSize(m_ui->graphicsView->size());
}


void MainWindow::moveEvent(QMoveEvent* event)
{
    if(m_navigationDialog->isVisible())
    {
        QSize tbSize = m_toolbarDateLabel->size();
        QPoint pos = mapToGlobal(m_ui->mainToolBar->pos()) + m_toolbarDateLabel->pos() + QPoint(10, tbSize.height());
        m_navigationDialog->move(pos);
    }
    QMainWindow::moveEvent(event);
}


void MainWindow::slotOpenIcalFile()
{
    QFileDialog *dlg = new QFileDialog( this, "open ical file",
                                        "../testfiles/",
                                        "ical-dateien (*.ics)" );
    dlg->setFileMode( QFileDialog::ExistingFiles );
    dlg->setAcceptMode( QFileDialog::AcceptOpen );
    if( dlg->exec() == QDialog::Accepted )
    {
        QStringList fileNames = dlg->selectedFiles();
        m_icalImportDialog->show();
        m_icalImportDialog->setFilenames( fileNames );
    }
}


void MainWindow::slotImportFromFileFinished()
{
    //m_icalImportDialog->hide();

    // generated data is in the buffer of thread
    for( const ThreadInfo ti : m_icalImportDialog->m_threads )
    {
        for( const Appointment* app : ti.thread->m_appointments )
        {
            // @fixme: appointments have an invalid? calendar id.
            m_storage->updateAppointment( (*app) );
        }
    }
    // delete threads
    m_icalImportDialog->deleteThreadsAndData();
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
            //m_scene->setSettings(m_settingsManager->currentSettings());
            //slotSetDate(m_scene->date());  // update appointments and toolbar view, needed if SettingsData::m_weekStartDay was modified
        }
    }
}


void MainWindow::slotAddUserCalendarDlg()
{
    m_userCalendarNewDialog->reset();
    m_userCalendarNewDialog->setUserCalendarInfos(m_userCalendarPool->calendarInfos());
    m_userCalendarNewDialog->show();
}


void MainWindow::slotAddUserCalendarDlgFinished(int returncode)
{
    if( returncode == QDialog::Accepted )
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


void MainWindow::slotCalendarManagerDialog()
{
    CalendarManagerDialog* dlg =
            new CalendarManagerDialog(m_userCalendarPool->calendarInfos(), this);
    connect(dlg, SIGNAL(signalModifyCalendar(int,QString,QColor)), this, SLOT(slotModifyCalendar(int,QString,QColor)));
    connect(dlg, SIGNAL(signalDeleteCalendar(int)), this, SLOT(slotDeleteCalendar(int)));
    dlg->exec();
}


/* User changes title and color, but not visibility.
 * connected in MainWindow::slotCalendarManagerDialog() */
void MainWindow::slotModifyCalendar(const int calendarId, const QString & title, const QColor & color)
{
    QColor oldcolor = m_userCalendarPool->color( calendarId );
    if(oldcolor != color)
    {
        m_eventPool->changeColor( calendarId, color );
        m_scene->eventsHaveNewColor( calendarId, color );
    }
    bool visible = m_userCalendarPool->isVisible(calendarId);
    m_storage->userCalendarDataModified(calendarId, color, title, visible);
}


/* User wants to delete a calendar. Delete it in m_userCalendarPool and m_storage.
 * connected in MainWindow::slotCalendarManagerDialog() */
void MainWindow::slotDeleteCalendar(const int calendarId)
{
    m_userCalendarPool->removeUserCalendar(calendarId);
    m_storage->removeUserCalendar(calendarId);
    showAppointments(m_scene->date());
}


/* Double Click or return pressed on a selected date in the small calendar or item in calendar view.
 * Start the AppointmentDialog and let the user create a new appointment.
 * - Note: The Appointmentdialog is non-modal, so it does not block. We just call
 * - show() and hide()
 * date parameter defaults to currentDate(), so this method is used as a menu slot too. */
void MainWindow::slotAppointmentDlgStart(const QDate &date)
{
    m_appointmentDialog->reset( date );
    QList<UserCalendarInfo*> uciList = m_userCalendarPool->calendarInfos();
    if(uciList.count() == 0)
        return;
    m_appointmentDialog->setUserCalendarInfos(uciList);
    m_appointmentDialog->createNewAppointment();
    m_appointmentDialog->show();
}


/* User wants to reconfigure an appointment. Take the data from this appointment
 * and show the appointmet dialog with the given data.
 * Communicate over appointment id with slotAppointmentDlgFinished(). */
void MainWindow::slotReconfigureAppointment( QString appointmentId )
{
    qDebug() << "MainWindow::slotReconfigureAppointment";
    QList<UserCalendarInfo*> uciList = m_userCalendarPool->calendarInfos();
    m_appointmentDialog->setUserCalendarInfos( uciList );

    if( m_eventPool->haveAppointment( appointmentId ) )
    {
        m_appointmentDialog->setAppointmentValues( m_eventPool->appointment( appointmentId) );
        m_appointmentDialog->show();
    }
}


/* Appointment dialog finishes, user has set up a new appointment or modified
 * an existing one. If user clicks on "OK", we add the appointment to the
 * database or change the existing item. */
void MainWindow::slotAppointmentDlgFinished(int returncode)
{
    if( returncode == QDialog::Rejected )
    {
        if( m_appointmentDialog->isNewAppointment() )
            m_appointmentDialog->deleteAppointment();
        m_appointmentDialog->hide();
        qDebug() << "MainWindow::slotAppointmentDlgFinished -> rejected";
        return;
    }
    // an in-use appointment,
    if( not ( m_appointmentDialog->modified() or m_appointmentDialog->isNewAppointment() ) )
    {
        m_appointmentDialog->hide();
        qDebug() << "MainWindow::slotAppointmentDlgFinished -> not modified";
        return;
    }

    // returncode == QDialog::Accepted

    // collect user data
    m_appointmentDialog->collectAppointmentData();
    Appointment* a = m_appointmentDialog->appointment();
    // make events
    a->makeEvents();
    a->setEventColor( m_userCalendarPool->color( a->m_userCalendarId ) );

    if( m_appointmentDialog->isNewAppointment() )
    {
        // write to db
        m_storage->storeAppointment( (*a) );
        // push to eventpool
        m_eventPool->addAppointment( a );
    }
    else
    {
        m_scene->removeAllEvents();
        // write to db
        m_storage->updateAppointment( (*a) );
        // push to eventpool
        m_eventPool->updateAppointment( a );
    }
    // show
    showAppointments( m_settingsManager->startDate() );
    m_appointmentDialog->hide();
}
