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
#include "usercalendarnew.h"
#include "ui_usercalendarnew.h"

#include <QDebug>


UserCalendarNew::UserCalendarNew(QWidget* parent) :
    QDialog(parent), m_ui(new Ui::UserCalendarNew)
{
    m_ui->setupUi(this);
    m_ui->treeCalInfo->setColumnCount(2);
    setupColorCombo();
    m_ui->comboStorage->addItem("Local");
    setModal(true);
}


UserCalendarNew::~UserCalendarNew()
{
    delete m_ui;
}


QString UserCalendarNew::calendarTitle() const
{
    return m_ui->leTitle->text();
}


QColor UserCalendarNew::calendarColor() const
{
    QString colorText = m_ui->comboColor->currentText();
    return QColor(colorText);
}


void UserCalendarNew::reset()
{
    m_ui->leTitle->clear();
    m_ui->treeCalInfo->clear();
}


void UserCalendarNew::setUserCalendarInfos(const QList<UserCalendarInfo*> uciList)
{
    // set up the calendar title
    int newCalNumber = 1;
    QRegExp title_pattern("New Calendar (\\d+)");
    for(UserCalendarInfo* uci : uciList)
    {
        QString title = uci->m_title;
        int pos = title_pattern.indexIn(title);
        if(pos > -1)
        {
            bool ok;
            int value = title_pattern.cap(1).toInt(&ok);
            if(ok)
                newCalNumber = value >= newCalNumber ? value + 1 : newCalNumber;
        }
    }
    QString newTitle = QString("New Calendar %1").arg(newCalNumber);
    m_ui->leTitle->setText(newTitle);

    // available calendars
    for(UserCalendarInfo* uci : uciList)
    {
        QTreeWidgetItem* item = new QTreeWidgetItem(m_ui->treeCalInfo);
        QPixmap pix(16, 16);
        pix.fill(uci->m_color);
        item->setIcon(0, pix);
        item->setText(1, uci->m_title);
    }
}


void UserCalendarNew::setupColorCombo()
{
    for(QString cName : QColor::colorNames())
    {
        // exclude black and white
        if(cName ==  "black" or cName == "white") continue;
        QPixmap pix(16, 16);
        pix.fill({cName});
        m_ui->comboColor->addItem(pix, cName);
    }
}
