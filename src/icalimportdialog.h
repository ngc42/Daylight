#ifndef ICALIMPORTDIALOG_H
#define ICALIMPORTDIALOG_H

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
    Ui::IcalImportDialog *m_ui;
    void parseIcalFile( const QString inFilename, QStringList &inContentLines );



signals:
    void sigFinishReadingFiles();

};

#endif // ICALIMPORTDIALOG_H