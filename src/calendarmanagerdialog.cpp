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
#include "calendarmanagerdialog.h"
#include "ui_calendarmanagerdialog.h"

#include <QDebug>

CalendarManagerDialog::CalendarManagerDialog(const QList<UserCalendarInfo*> & calList, QWidget* parent) :
    QDialog(parent),
    m_ui(new Ui::CalendarManagerDialog),
    m_calListCopy(calList)
{
    m_ui->setupUi(this);

    // colors for manage calendars
    for(QString cName : QColor::colorNames())
    {
        // exclude black and white
        if(cName ==  "black" or cName == "white") continue;
        QPixmap pix(20, 20);
        pix.fill({cName});
        m_ui->cbColor->addItem(pix, cName);
    }
    showUserCalendars();

    connect(m_ui->pbModifyCalendar, SIGNAL(clicked()), this, SLOT(slotModifyCalendar()));
    connect(m_ui->pbDeleteCalendar, SIGNAL(clicked()), this, SLOT(slotDeleteCalendar()));
    connect(m_ui->lwManageCalendars, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(slotCurrentItemChanged(QListWidgetItem*,QListWidgetItem*)));

    m_ui->tabWidget->setCurrentIndex(0);
}


CalendarManagerDialog::~CalendarManagerDialog()
{
    delete m_ui;
}


void CalendarManagerDialog::showUserCalendars()
{
    m_ui->cbLeftCalendars->clear();
    m_ui->cbRightCalendars->clear();
    m_ui->lwManageCalendars->clear();

    for(UserCalendarInfo* uci : m_calListCopy)
    {
        QPixmap pix(20, 20);
        pix.fill(uci->m_color);

        // manage calendars page
        QString title(uci->m_title);
        m_ui->cbLeftCalendars->addItem(pix, title, uci->m_id);
        m_ui->cbRightCalendars->addItem(pix, title, uci->m_id);

        // manage calendars page
        QListWidgetItem* mngWidgetItem = new QListWidgetItem(pix, title);
        mngWidgetItem->setData(Qt::UserRole, uci->m_id);
        m_ui->lwManageCalendars->addItem(mngWidgetItem);
    }

    m_ui->lIdView->setText("");
    m_ui->leTitle->clear();
}


void CalendarManagerDialog::slotModifyCalendar()
{
    QListWidgetItem* itm = m_ui->lwManageCalendars->currentItem();
    if(itm == nullptr) return;      // no calendar was selected, return
    bool ok;
    int calId = itm->data(Qt::UserRole).toInt(&ok);
    qDebug() << "CalendarManagerDialog::slotModifyCalendar() id=" << calId;
    QString title = m_ui->leTitle->text();
    QColor color = QColor(m_ui->cbColor->currentText());
    for(UserCalendarInfo* uci : m_calListCopy)
    {
        if(uci->m_id == calId)
        {
            uci->m_title = title;
            uci->m_color = color;
        }
    }
    showUserCalendars();
    emit signalModifyCalendar(calId, title, color);
}


/* Button "Delete Calendar" was clicked. Get id of this calendar and
 * send delete signal.
 * fixme: asking user in case of available appointments */
void CalendarManagerDialog::slotDeleteCalendar()
{
    QListWidgetItem* itm = m_ui->lwManageCalendars->currentItem();
    if(itm == nullptr) return;      // no calendar was selected, return
    bool ok;
    int calId = itm->data(Qt::UserRole).toInt(&ok);
    qDebug() << "CalendarManagerDialog::slotDeleteCalendar() id=" << calId;
    m_ui->lwManageCalendars->removeItemWidget(itm);
    for(UserCalendarInfo* uci : m_calListCopy)
    {
        if(uci->m_id == calId)
        {
            m_calListCopy.removeOne(uci);
            delete uci;
        }
    }
    delete itm;
    showUserCalendars();
    emit signalDeleteCalendar(calId);
}


void CalendarManagerDialog::slotCurrentItemChanged(QListWidgetItem* currentItem, QListWidgetItem*)
{
    bool ok;
    QColor c;
    int calIdCurrent = currentItem == nullptr ? 0 : currentItem->data(Qt::UserRole).toInt(&ok);
    for(UserCalendarInfo* uci : m_calListCopy)
    {
        if(calIdCurrent == uci->m_id)
        {
            m_ui->lIdView->setText(QString("%1").arg(uci->m_id));
            m_ui->leTitle->setText(uci->m_title);
            // set the color combo box
            QColor uci_color = uci->m_color;
            for(QString colorName : QColor::colorNames())
            {
                c.setNamedColor(colorName);
                if(c  == uci_color)
                {
                    m_ui->cbColor->setCurrentText(colorName);
                }
            }
            break;
        }
    }
}


