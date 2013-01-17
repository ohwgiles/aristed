#ifndef FILESEARCHDIALOG_HPP
#define FILESEARCHDIALOG_HPP

#include <QDialog>

namespace Ui {
class AeFileSearchDialog;
}

class AeFileSearchDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit AeFileSearchDialog(QWidget *parent = 0);
	~AeFileSearchDialog();
	
private:
	Ui::AeFileSearchDialog *ui;
};

#endif // FILESEARCHDIALOG_HPP
