#ifndef FILESEARCHDIALOG_HPP
#define FILESEARCHDIALOG_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QDialog>
#include <QStringListModel>

class UneditableStringListModel : public QStringListModel {
public:
	Qt::ItemFlags flags(const QModelIndex &index) const {
		return QAbstractItemModel::flags(index);
	}
};

namespace Ui {
class AeFileSearchDialog;
}
class AeProject;
class AeFileSearchDialog : public QDialog
{
	Q_OBJECT
	
public:
	explicit AeFileSearchDialog(const AeProject *project, QWidget *parent = 0);
	~AeFileSearchDialog();
	QString fileToOpen() const;

protected:
	bool eventFilter(QObject * o, QEvent * event);

private slots:
	void on_patternInput_textEdited(const QString &arg1);
	void resultSelected(QModelIndex current, QModelIndex previous);
	void on_resultFiles_activated(const QModelIndex &index);

private:
	Ui::AeFileSearchDialog *ui;
	UneditableStringListModel model_;
	QStringList items_;
	const AeProject* project_;
};

#endif // FILESEARCHDIALOG_HPP
