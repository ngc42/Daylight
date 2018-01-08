#ifndef ICALIMPORTDIALOG_H
#define ICALIMPORTDIALOG_H

#include <QDialog>

namespace Ui {
    class IcalImportDialog;
}

class IcalImportDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IcalImportDialog(QWidget *parent = 0);
    ~IcalImportDialog();

private:
    Ui::IcalImportDialog *ui;
};

#endif // ICALIMPORTDIALOG_H
