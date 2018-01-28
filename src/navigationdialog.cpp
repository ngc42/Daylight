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
#include "navigationdialog.h"
#include <QVBoxLayout>


NavigationDialog::NavigationDialog(QWidget *parent) :
    QDialog(parent, Qt::FramelessWindowHint|Qt::Tool)
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout(layout);
    m_calendarWidget = new QCalendarWidget(this);
    layout->addWidget(m_calendarWidget);
    m_calendarWidget->setSelectedDate(QDate::currentDate());
    connect(m_calendarWidget, SIGNAL(activated(QDate)), this, SIGNAL(signalActivated(QDate)));
    connect(m_calendarWidget, SIGNAL(clicked(QDate)), this, SIGNAL(signalClicked(QDate)));
    connect(m_calendarWidget, SIGNAL(currentPageChanged(int, int)), this, SIGNAL(signalCurrentPageChanged(int, int)));
}


NavigationDialog::~NavigationDialog()
{
    disconnect(m_calendarWidget, SIGNAL(activated(QDate)), this, SIGNAL(signalActivated(QDate)));
    disconnect(m_calendarWidget, SIGNAL(clicked(QDate)), this, SIGNAL(signalClicked(QDate)));
    disconnect(m_calendarWidget, SIGNAL(currentPageChanged(int, int)), this, SIGNAL(signalCurrentPageChanged(int, int)));
    delete m_calendarWidget;
}


void NavigationDialog::slotSetSelectedDate(const QDate & date)
{
    // disconnect, just to prevent that unnecessary MainWindow::setDate() events occur
    disconnect(m_calendarWidget, SIGNAL(currentPageChanged(int, int)), this, SIGNAL(signalCurrentPageChanged(int, int)));
    m_calendarWidget->setSelectedDate(date);
    connect(m_calendarWidget, SIGNAL(currentPageChanged(int, int)), this, SIGNAL(signalCurrentPageChanged(int, int)));
}
