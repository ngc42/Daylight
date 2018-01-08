#include <QFile>
#include "ui_icalimportdialog.h"
#include "icalimportdialog.h"
#include "../icalreader/icalbody.h"
#include "../icalreader/icalinterpreter.h"


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
    m_ui->pBarFiles->reset();
    m_ui->pBarEvents->reset();
    m_ui->pBarFiles->setMaximum( inList.count() );
    m_ui->teContent->clear();
    m_ui->teMessages->clear();
    int currentNum = 0;
    for( const QString fn : inList )
    {
        QFile file( fn );
        if( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
        {

            currentNum++;
            m_ui->pBarFiles->setValue( currentNum );
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
                m_ui->teContent->insertPlainText( s );
            }
            file.close();
            if( lineList.count() > 0 )
                parseIcalFile( fn, lineList );
        }
    }

    emit sigFinishReadingFiles();
}


void IcalImportDialog::parseIcalFile( const QString inFilename, QStringList &inContentLines )
{
    m_ui->pBarEvents->reset();
    ICalBody vcal;
    IcalInterpreter interpreter;

    QString dbg = QString( "<strong>parse: %1</strong>\n").arg( inFilename );
    m_ui->teMessages->insertHtml( dbg );

    bool startOfReadingCalfile = false;
    while( not inContentLines.isEmpty() )
    {
        QString contentLine = inContentLines.first();
        inContentLines.removeFirst();


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
        else
        {
            m_ui->teMessages->insertPlainText( contentLine.append( '\n') );
        }
    }
    if( vcal.validateIcal() )
        m_ui->teMessages->insertPlainText( "Validated!\n" );
    else
        m_ui->teMessages->insertPlainText( "Has Errors\n" );

}

/*
void MainWindow::parseVCalendarfile()
{
    AppointmentManager *manager = new AppointmentManager();
    interpreter.readIcal( vcal, manager );
    int num = 1;
    for( Appointment* t : manager->m_appointmentList )
    {
        QString s = QString( "==== Appointment #%1 ====\n").arg( num );
        num++;
        m_ui->textEdit_2->insertPlainText( s );
        if( t->m_appBasics )
            m_ui->textEdit_2->insertPlainText( t->m_appBasics->contententToString() );
        if( t->m_appRecurrence )
            m_ui->textEdit_2->insertPlainText( t->m_appRecurrence->contentToString() );
        for( const AppointmentAlarm* alarm : t->m_appAlarms )
            m_ui->textEdit_2->insertPlainText( alarm->contentToString() );
        s = QString( "---- Dates ----\n");
        m_ui->textEdit_2->insertPlainText( s );
        for( const Event e : t->m_eventList )
        {
            QString a = e.m_startDt.toString() + " - " + e.m_endDt.toString() + '\n';
            m_ui->textEdit_2->insertPlainText( a );
        }

    }
}


void MainWindow::slotParseEditorText()
{
    m_ui->listWidget->clear();
    m_ui->textEdit_2->clear();
    parseVCalendarfile();
}
*/

