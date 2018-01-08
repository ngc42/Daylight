#include "icalimportdialog.h"
#include "ui_icalimportdialog.h"


IcalImportDialog::IcalImportDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::IcalImportDialog)
{
    ui->setupUi(this);
}


IcalImportDialog::~IcalImportDialog()
{
    delete ui;
}
