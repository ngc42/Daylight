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
#include <QMessageBox>
#include "ui_mainwindow.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent), m_ui(new Ui::MainWindow)
{
    m_ui->setupUi(this);
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

    // user calendar pool
    m_userCalendarPool = new UserCalendarPool(this);

    // new user calendar
    m_userCalendarNewDialog = new UserCalendarNew(this);
    m_userCalendarNewDialog->hide();

    // connect main signals
    connect(m_ui->actionExit, SIGNAL(triggered()), qApp, SLOT(quit()));

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
    m_storage->loadAppointmentData(2018);
    disconnect(m_storage, SIGNAL(signalLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)),
               this, SLOT(slotLoadedAppointmentFromStorage(AppointmentData, RecurrenceData)));

    QMenu* tmp = m_userCalendarPool->calendarMenu();
    m_toolbarUserCalendarMenu->setMenu(tmp);
    connect(m_userCalendarPool, SIGNAL(signalUserCalendarDataModified(int,QColor,QString,bool)),
            m_storage, SLOT(slotUserCalendarDataModified(int,QColor,QString,bool)));
    // - end storage

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
    QSize tbSize = m_toolbarDateLabel->size();
    QPoint pos = mapToGlobal(m_ui->mainToolBar->pos()) + m_toolbarDateLabel->pos() + QPoint(10, tbSize.height());

    QMainWindow::moveEvent(event);
}

void MainWindow::slotLoadedAppointmentFromStorage(const Appointment &apmData)
{
    QColor color = m_userCalendarPool->color(apmData.m_userCalendarId);
}

