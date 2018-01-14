/*  Daylight - day planner, appointment book application
    Copyright (C) 2014-2018  Eike Lange (eike(at)ngc42.de)

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
#ifndef ICALIMPORTTHREAD_H
#define ICALIMPORTTHREAD_H

#include <QStringList>
#include <QThread>
#include <QVector>
#include <QDateTime>

#include "appointmentmanager.h"
#include "../icalreader/icalbody.h"
#include "../icalreader/icalinterpreter.h"


/* Import thread reads a given Ical-File and creates appointment Data out of it.
 * This is a threaded version controlled by someone, who splitted the file already
 *  into content lines, where follow-up lines are merged.
 * The Appointment data is then ready in m_appointments.
 *
 * There are several information services generated for the outside world:
 *  - sigTickEvent generates a progress counter for each generated Event.
 *  - sigTickVEvent generates a progress counter for each read VEVENT.
 *  both signals are just forwarded from the underlying icalinterpreter but
 *   tagged withe the thread id
 *
 *  - sigThreadFinished - thread is finished generating Appointments
 *  - sigWeDislikeIcalFile - there is something wrong with the ical file
 */

class IcalImportThread : public QThread
{
    Q_OBJECT

public:
    // reasons to dislike the ical file
    enum IcalDislikeReasonType {
        DOES_NOT_VALIDATE
    };

    // constructor, reads ical file as content lines.
    explicit IcalImportThread( const int inThreadId, const QStringList &inContentLines, QObject* parent = Q_NULLPTR );

    // fires up the thread generating Events
    void run() override;

    // Thats wht we get: Appointments
    QVector<Appointment*>   m_appointments;

private:
    int             m_threadId;
    QStringList     m_contentLines;

signals:
    // an event was generated
    void sigTickEvent( const int threadID, const int min, const int current, const int max );

    // a VEVENT was read
    void sigTickVEvents( const int threadID, const int min, const int current, const int max );

    // we are finished
    void sigThreadFinished( const int threadID );

    // something wrong with the ical
    void sigWeDislikeIcalFile( const int threadId, const IcalImportThread::IcalDislikeReasonType reason );

public slots:
    void slotTickEvent( const int min, const int current, const int max );
    void slotTickVEvents( const int min, const int current, const int max );
    // get an appointment and store this in m_appointments
    void slotAppointmentReady( Appointment* app );
    void slotThreadFinished();
};

#endif // ICALIMPORTTHREAD_H
