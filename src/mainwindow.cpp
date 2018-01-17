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
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
    m_settingsManager = new SettingsManager();
    m_storage = new Storage(this);
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

    // in-app database
    m_eventPool = new EventPool();

    // user calendar pool
    m_userCalendarPool = new UserCalendarPool(this);

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

    // user calendars
    connect(m_ui->actionAddUserCalendar, SIGNAL(triggered()), this, SLOT(slotAddUserCalendar()));
    connect(m_userCalendarNewDialog, SIGNAL(finished(int)), this, SLOT(slotAddUserCalendarDlgFinished(int)));

    // storage load data
    connect(m_storage, SIGNAL(signalLoadedUserCalendarFromStorage(UserCalendarInfo* &)),
            m_userCalendarPool, SLOT(slotAddUserCalendarFromStorage(UserCalendarInfo* &)));
    m_storage->loadUserCalendarInfo();
    disconnect(m_storage, SIGNAL(signalLoadedUserCalendarFromStorage(UserCalendarInfo* &)),
            m_userCalendarPool, SLOT(slotAddUserCalendarFromStorage(UserCalendarInfo* &)));
    connect(m_storage, SIGNAL(signalLoadedAppointmentFromStorage(Appointment*)),
            this, SLOT(slotLoadedAppointmentFromStorage(Appointment*)));
    m_storage->loadAppointmentByYear( QDateTime::currentDateTime().date().year() );
    m_eventPool->addMarker( QDateTime::currentDateTime().date().year() );
    disconnect(m_storage, SIGNAL(signalLoadedAppointmentFromStorage(Appointment*)),
               this, SLOT(slotLoadedAppointmentFromStorage(Appointment*)));
    QMenu* tmp = m_userCalendarPool->calendarMenu();
    m_toolbarUserCalendarMenu->setMenu(tmp);
    connect(m_userCalendarPool, SIGNAL(signalUserCalendarDataModified(int,QColor,QString,bool)),
            m_storage, SLOT(slotUserCalendarDataModified(int,QColor,QString,bool)));
    // - end storage

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
    //slotSetDate( m_settingsManager->startDate() );

}


MainWindow::~MainWindow()
{
    delete m_ui;
}

void MainWindow::resizeEvent(QResizeEvent* )
{
}


void MainWindow::moveEvent(QMoveEvent* event)
{
    //QSize tbSize = m_toolbarDateLabel->size();
    //QPoint pos = mapToGlobal(m_ui->mainToolBar->pos()) + m_toolbarDateLabel->pos() + QPoint(10, tbSize.height());

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


void MainWindow::slotLoadedAppointmentFromStorage( Appointment* apmData )
{
    m_eventPool->addAppointment( apmData );
    qDebug() << " loadAppointment " << apmData->m_uid;
    //QColor color = m_userCalendarPool->color(apmData.m_userCalendarId);
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

void MainWindow::slotAddUserCalendar()
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


