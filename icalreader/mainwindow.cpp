/*  This file is part of Kalendar-Reader.
Kalendar-Reader is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kalendar-Reader is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
*/



void MainWindow::parseVCalendarfile()
{
    QString content = m_ui->textEdit->toPlainText();
    if( content.isEmpty() )
        return;
    QStringList contentList = content.split( '\n' );

    ICalBody vcal;
    IcalInterpreter interpreter;

    bool startOfReadingCalfile = false;
    while( not contentList.isEmpty() )
    {
        QString contentLine = contentList.first();
        contentList.removeFirst();

        while( not contentList.isEmpty() )
        {
            // care for follow-up lines
            QString followedLine = contentList.first();
            if( followedLine.startsWith( ' ' ) )
            {
                followedLine = followedLine.right( followedLine.length() - 1 );    // remove space
                contentLine = contentLine.append( followedLine );
                contentList.removeFirst();
            }
            else
                break;  // no (more) follow-up lines
        }

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
            QListWidgetItem *item = new QListWidgetItem( contentLine );
            item->setBackground( Qt::red );
            m_ui->listWidget->addItem( item );
        }
    }

    if( vcal.validateIcal() )
        m_ui->textEdit_2->insertPlainText( "Works!\n" );
    else
        m_ui->textEdit_2->insertPlainText( "Works not\n" );

    QStringList vcalList = vcal.getContent().split( '\n' );
    for( QString elem : vcalList )
    {
        QListWidgetItem *item = new QListWidgetItem( elem );
        item->setBackground( Qt::green );
        m_ui->listWidget->addItem( item );
    }


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


void MainWindow::slotFileOpenDialog()
{
    QFileDialog *dlg = new QFileDialog( this, "open ical file",
        "../testfiles/",
        "ical-dateien (*.ics)" );
    dlg->setFileMode( QFileDialog::ExistingFile );
    dlg->setAcceptMode( QFileDialog::AcceptOpen	);

    if( dlg->exec() == QDialog::Accepted )
    {
        QStringList fileNames = dlg->selectedFiles();
        if( fileNames.count() == 1 )
        {
            QFile file( fileNames.at( 0 ) );
            if( file.open( QIODevice::ReadOnly | QIODevice::Text ) )
            {
                m_ui->textEdit->clear();
                m_ui->textEdit_2->clear();
                while( ! file.atEnd() )
                {
                    QString s = file.readLine();
                    m_ui->textEdit->insertPlainText( s );
                    m_ui->listWidget->clear();
                }
                file.close();
            }
        }
    }
}


void MainWindow::slotParseEditorText()
{
    m_ui->listWidget->clear();
    m_ui->textEdit_2->clear();
    parseVCalendarfile();
}
