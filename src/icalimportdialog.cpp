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
#include <QFile>
#include "ui_icalimportdialog.h"
#include "icalimportdialog.h"


IcalImportDialog::IcalImportDialog( QWidget *parent ) :
    QDialog( parent ),
    m_ui( new Ui::IcalImportDialog )
{
    m_ui->setupUi( this );
}


IcalImportDialog::~IcalImportDialog()
{
    delete m_ui;
}


void IcalImportDialog::setFilenames( QStringList &inList )
{
    m_ui->pBarVEvents->reset();
    m_ui->pBarEvents->reset();
    m_ui->teContent->clear();
    m_ui->teMessages->clear();
    deleteThreadsAndData();
    int currentNum = 0;
    for( const QString fn : inList )
    {
        QFile file( fn );
        if( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {

            currentNum++;
            QStringList lineList;
            while( not file.atEnd() )
            {
                QString s = file.readLine();
                if( s.isEmpty() )
                    continue;
                s.chop(1);          // remove last character '\n'
                if( s.startsWith( ' ' ) )
                {   // follow-up lines
                    QString lastLine = lineList.last();
                    lineList.removeLast();
                    lastLine = lastLine.append( s.right(s.count() - 1 ) );
                    lineList.append( lastLine );
                    continue;
                }
                lineList.append( s );
                m_ui->teContent->insertPlainText( s.append( '\n' ) );
            }
            file.close();
            if( lineList.count() > 0 )
                parseIcalFile( fn, lineList );
        }
    }
}

void IcalImportDialog::deleteThreadsAndData()
{
    for( const ThreadInfo ti : m_threads )
    {
        ti.thread->deleteLater();
    }
    m_threads.clear();
}


void IcalImportDialog::parseIcalFile( const QString inFilename, QStringList &inContentLines )
{
    m_ui->pBarEvents->reset();

    ThreadInfo t;
    t.thread = new IcalImportThread( m_threads.count(), inContentLines, this );
    t.filename = inFilename;
    t.v_min = 0, t.v_current = 0, t.v_max = 0;
    t.e_min = 0, t.e_current = 0, t.e_max = 0;
    t.ended = false;
    t.successful = true;
    m_threads.append( t );

    connect( t.thread, SIGNAL(sigTickEvent(int,int,int,int)),
             this, SLOT(slotTickEvent(int,int,int,int)) );
    connect( t.thread, SIGNAL(sigTickVEvents(int,int,int,int)),
             this, SLOT(slotTickVEvents(int,int,int,int)) );
    connect( t.thread, SIGNAL(sigThreadFinished(int)),
             this, SLOT(slotThreadFinished(int)) );
    connect( t.thread, SIGNAL(sigWeDislikeIcalFile(int,int)),
             this, SLOT(slotWeDislikeIcalFile(int,int)) );
    t.thread->start();
}


void IcalImportDialog::displayContentToMessage()
{
    for( const ThreadInfo mi : m_threads )
    {
        if( mi.successful )
        {
            m_ui->teMessages->insertPlainText( QString( "=== %1 ===\n" ).arg(mi.filename) );

            for( const Appointment* app : mi.thread->m_appointments )
            {
                m_ui->teMessages->insertPlainText(
                            app->m_appBasics->contententToString() );
            }
        }
    }
}


void IcalImportDialog::slotTickEvent( const int threadId, int min, int current, int max )
{
    m_threads[threadId].e_min = min;
    m_threads[threadId].e_current = current;
    m_threads[threadId].e_max = max;
    int smin = 0, scurrent = 0, smax = 0;
    for( const ThreadInfo ti : m_threads )
    {
        smin += ti.e_min;
        scurrent += ti.e_current;
        smax += ti.e_max;
    }
    m_ui->pBarEvents->setRange( smin, smax );
    m_ui->pBarEvents->setValue( scurrent );
}


void IcalImportDialog::slotTickVEvents( const int threadId, int min, int current, int max )
{
    m_threads[threadId].v_min = min;
    m_threads[threadId].v_current = current;
    m_threads[threadId].v_max = max;

    int smin = 0, scurrent = 0, smax = 0;
    for( const ThreadInfo ti : m_threads )
    {
        smin += ti.v_min;
        scurrent += ti.v_current;
        smax += ti.v_max;
    }
    m_ui->pBarVEvents->setRange( smin, smax );
    m_ui->pBarVEvents->setValue( scurrent );
}


void IcalImportDialog::slotThreadFinished( const int threadId )
{
    qDebug() << "I should delete " << threadId ;
    m_threads[threadId].ended = true;
    if( m_threads[threadId].successful )
        m_ui->teMessages->insertPlainText(
                    QString( "* OK: %1 finished\n" )
                    .arg( m_threads[threadId].filename ) );
    bool allThreadsAreFinished = true;
    for( const ThreadInfo ti : m_threads )
    {
        if( not ti.ended )
        {
            allThreadsAreFinished = false;
            break;
        }
    }
    if( allThreadsAreFinished )
    {
        emit sigFinishReadingFiles();
        qDebug() << "Threads are finished";
        displayContentToMessage();
    }
}


void IcalImportDialog::slotWeDislikeIcalFile( const int threadId, const int reason )
{
    m_threads[threadId].successful = false;
    if( static_cast<IcalImportThread::IcalDislikeReasonType>(reason) == IcalImportThread::IcalDislikeReasonType::DOES_NOT_VALIDATE )
        m_ui->teMessages->insertPlainText(
                    QString( "* ERR: %1 does not validate\n" )
                    .arg( m_threads[threadId].filename ) );
}
