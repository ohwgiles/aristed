#include <QRegExp>
#include <QKeyEvent>
#include <QDirIterator>
#include "project.hpp"
#include "filesearchdialog.hpp"
#include "ui_filesearchdialog.h"
#include "log.hpp"




QString AeFileSearchDialog::fileToOpen() const {
    return model_.data(ui->resultFiles->currentIndex(), Qt::DisplayRole).toString();
}

bool AeFileSearchDialog::eventFilter(QObject * o, QEvent * event) {
	if(o == ui->patternInput && event->type() == QEvent::KeyPress) {
		QKeyEvent* e = (QKeyEvent*) event;
		if(e->key() == Qt::Key_Down || (e->key() == Qt::Key_J && (e->modifiers() & Qt::ALT))) {
			ui->resultFiles->setCurrentIndex(model_.index(ui->resultFiles->currentIndex().row()+1));
			return true;
		}
		if(e->key() == Qt::Key_Up || (e->key() == Qt::Key_K && (e->modifiers() & Qt::ALT))) {
			ui->resultFiles->setCurrentIndex(model_.index(ui->resultFiles->currentIndex().row()-1));
			return true;
		}
	}
	return false;
}

AeFileSearchDialog::AeFileSearchDialog(const AeProject *project, QWidget *parent) :
   QDialog(parent),
	ui(new Ui::AeFileSearchDialog)
	//model_(new FilePathStringModel(ui->resourcePath))
{
	ui->setupUi(this);
    QDirIterator iterator(project->sourceDir().absolutePath(), QDirIterator::Subdirectories);
    while (iterator.hasNext()) {
       iterator.next();
       if (!iterator.fileInfo().isDir()) {
          items_.append(iterator.filePath());
       }
    }
	ui->resultFiles->setModel(&model_);
	ui->resultFiles->setFocusProxy(ui->patternInput);
	ui->patternInput->installEventFilter(this);
	connect(ui->resultFiles->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(resultSelected(QModelIndex,QModelIndex)));
}

AeFileSearchDialog::~AeFileSearchDialog()
{
	delete ui;
}
void AeFileSearchDialog::resultSelected(QModelIndex current, QModelIndex ){
	QString file(model_.data(current,Qt::DisplayRole).toString());
	ae_debug("Selected item " << file);
    ui->resourcePath->setText(file);
}

void AeFileSearchDialog::on_patternInput_textEdited(const QString &txt)
{
	if(txt.length() < 3) {
		model_.setStringList(QStringList());
		ui->resultFiles->reset();
		return;
	}
	QRegExp filter(txt);
	model_.setStringList(items_.filter(filter));
	ui->resultFiles->setCurrentIndex(model_.index(0));
}

void AeFileSearchDialog::on_resultFiles_activated(const QModelIndex &) {
	close();
}
