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

private:
    Ui::IcalImportDialog*   m_ui;
    QVector<ThreadInfo>     m_threads;
    void parseIcalFile( const QString inFilename, QStringList &inContentLines );
    void displayContentToMessage();


signals:
    void sigFinishReadingFiles();

private slots:
    void slotTickEvent( const int id, int min, int current, int max );
    void slotTickVEvents( const int id, int min, int current, int max );
    void slotThreadFinished( const int id );
    void slotWeDislikeIcalFile( const int threadId, const IcalImportThread::IcalDislikeReasonType reason );
};

#endif // ICALIMPORTDIALOG_H
