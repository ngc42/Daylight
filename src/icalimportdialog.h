#ifndef ICALIMPORTDIALOG_H
#define ICALIMPORTDIALOG_H

#include "appointmentmanager.h"
#include "../icalreader/icalinterpreter.h"


#include <QDebug>
#include <QDialog>
#include <QStringList>

namespace Ui {
    class IcalImportDialog;
}

class IcalImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IcalImportDialog(QWidget *parent = 0);
    ~IcalImportDialog();

    void setFilenames( QStringList &inList );

private:
    Ui::IcalImportDialog*   m_ui;
    IcalInterpreter*        m_icalInterpreter;
    void parseIcalFile( const QString inFilename, QStringList &inContentLines );

signals:
    void sigFinishReadingFiles();

private slots:
    void slotReceiveAppointment( Appointment* const &appointment );
    void slotTick( int first, int current, int last );
    void slotInterpreterFinished();
};

#endif // ICALIMPORTDIALOG_H
