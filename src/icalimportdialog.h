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
#ifndef ICALIMPORTDIALOG_H
#define ICALIMPORTDIALOG_H

#include "appointmentmanager.h"
#include "icalimportthread.h"

#include <QDebug>
#include <QDialog>
#include <QStringList>
#include <QVector>

namespace Ui {
    class IcalImportDialog;
}

struct ThreadInfo
{
    IcalImportThread* thread;
    QString filename;
    // processing VEvents inside of thread, used for progressbar
    int v_min, v_current, v_max;
    // generating Events inside of thread, used for progressbar
    int e_min, e_current, e_max;
    // end of thread
    bool ended;
    // unsuccessful: false
    bool successful;
};

class IcalImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IcalImportDialog(QWidget *parent = 0);
    ~IcalImportDialog();
    void setFilenames( QStringList &inList );
    void deleteThreadsAndData();

    QVector<ThreadInfo>     m_threads;


private:
    Ui::IcalImportDialog*   m_ui;
    void parseIcalFile( const QString inFilename, QStringList &inContentLines );
    void displayContentToMessage();

signals:
    void sigFinishReadingFiles();

private slots:
    void slotTickEvent( const int id, int min, int current, int max );
    void slotTickVEvents( const int id, int min, int current, int max );
    void slotThreadFinished( const int id );
    void slotWeDislikeIcalFile( const int threadId, const int reason );
};

#endif // ICALIMPORTDIALOG_H
