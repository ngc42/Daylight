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
#include "icalimportthread.h"

#include <QDebug>


IcalImportThread::IcalImportThread(const int inThreadId, const QStringList &inContentLines, QObject* parent )
    :
      QThread(parent),
      m_threadId(inThreadId),
      m_contentLines( inContentLines )
{
    // we connect this insode of this class, because we want append
    // the threadID
    connect( this, SIGNAL(finished()), this, SLOT(slotThreadFinished()) );
}


void IcalImportThread::run()
{
    ICalBody vcal;
    IcalInterpreter interpreter;

    // read content lines and build up ICalBody
    bool startOfReadingCalfile = false;
    while( not m_contentLines.isEmpty() )
    {
        QString contentLine = m_contentLines.first();
        m_contentLines.removeFirst();

        if( contentLine.compare( "BEGIN:VCALENDAR", Qt::CaseInsensitive ) == 0 )
        {
            startOfReadingCalfile = true;
            continue;
        }

        if( contentLine.compare( "END:VCALENDAR", Qt::CaseInsensitive ) == 0 )
        {
            startOfReadingCalfile = false;
            continue;
        }

        if( startOfReadingCalfile )
        {
            vcal.readContentLine( contentLine );
        }
    }

    // validate
    if( not vcal.validateIcal() )
    {
        emit sigWeDislikeIcalFile( m_threadId, static_cast<int>(IcalDislikeReasonType::DOES_NOT_VALIDATE) );
        return;
    }

    // interpret ical file and generate Events.
    // report progress to the outside world
    connect( &interpreter, SIGNAL(sigTickEvent(int,int,int)),
             this, SLOT(slotTickEvent(int,int,int)) );
    connect( &interpreter, SIGNAL(sigTickVEvents(int,int,int)),
             this, SLOT(slotTickVEvents(int,int,int)) );
    connect( &interpreter, SIGNAL( sigAppointmentReady(Appointment*)),
             this, SLOT( slotAppointmentReady(Appointment*)) );
    interpreter.readIcal( vcal );
}


void IcalImportThread::slotTickEvent( const int min, const int current, const int max )
{
    // append threadId
    emit sigTickEvent( m_threadId, min, current, max );
}


void IcalImportThread::slotTickVEvents( const int min, const int current, const int max )
{
    // append threadId
    emit sigTickVEvents( m_threadId, min, current, max );
}


void IcalImportThread::slotAppointmentReady(Appointment *app )
{
    // store the appointment
    m_appointments.append( app );
}


void IcalImportThread::slotThreadFinished()
{
    // append threadId
    emit sigThreadFinished( m_threadId );
}
