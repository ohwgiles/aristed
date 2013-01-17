#include "filesearchdialog.hpp"
#include "ui_filesearchdialog.h"

AeFileSearchDialog::AeFileSearchDialog(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::AeFileSearchDialog)
{
	ui->setupUi(this);
}

AeFileSearchDialog::~AeFileSearchDialog()
{
	delete ui;
}
