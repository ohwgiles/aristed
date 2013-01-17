#include "window.hpp"
#include "ui_mainwindow.h"

#include "editor.hpp"
#include "cxxmodel.hpp"
#include "colourscheme.hpp"
#include "log.hpp"
#include "project.hpp"
#include "filesearchdialog.hpp"

#include <QTabWidget>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QLabel>
#include <QFileSystemModel>

AeWindow::AeWindow(QList<AeProject *> &projects, QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mColourScheme(new colour::SolarizedDark()),
	projects_(projects)
{
	ui->setupUi(this);

	dirModel_ = new QFileSystemModel();
	ui->fileView->setModel(dirModel_);
	dirModel_->setRootPath(QDir::homePath());
	ui->fileView->setRootIndex(dirModel_->index(QDir::homePath()));
	ui->fileView->setColumnHidden(1, true);
	ui->fileView->setColumnHidden(2, true);
	ui->fileView->setColumnHidden(3, true);
	ui->fileView->header()->hide();
	ui->fileTree->hide();

	ui->console->hide();

	m_tabs = new TabWidget(this);
	QIcon icon(":icon.png");

	setWindowIcon(icon);

	this->setCentralWidget(m_tabs);
	connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));


	cursor_position = new QLabel();
	ui->statusbar->addPermanentWidget(cursor_position);

	on_actionNew_triggered();
}

void AeWindow::currentTabChanged(int index) {
	ae_info("Changed tab to " << index);
	if(index < 0) return;
	AeEditor* e = (AeEditor*) m_tabs->widget(index);

	setWindowTitle(e->displayName() + " - " + e->project()->displayName() + " - aristed");
	//dirModel_->setRootPath(e->project()->sourceDir().absolutePath());
	ui->fileView->setRootIndex(dirModel_->index(e->project()->sourceDir().absolutePath()));
	m_tabs->setTabText(index, e->displayName());
	ui->actionRevert_to_Saved->setEnabled(e->fileExists() && e->dirty());
	ui->actionDiff_to_Saved->setEnabled(e->fileExists() && e->dirty());
	ui->actionSave->setEnabled(e->dirty());
}

AeWindow::~AeWindow() {
	delete ui;
	delete m_tabs;
}

template<typename Model>
AeEditor* AeWindow::createEditor() {
	AeEditor* e = new AeEditor(this);
	e->setCxxModel();
	e->setColourScheme(mColourScheme);
	connect(e, SIGNAL(dirtied(QWidget*,bool)), this, SLOT(handleDirtied(QWidget*,bool)));
	connect(e->model(), SIGNAL(positionInfo(QString)), ui->statusbar, SLOT(showMessage(QString)));
	connect(e, SIGNAL(updateCursorPosition(QString)), cursor_position, SLOT(setText(QString)));

	return e;
}

void AeWindow::on_actionNew_triggered() {
	AeEditor * e = createEditor<AeCxxModel>();
	AeProject* p = AeProject::getProject(projects_, "");
	e->setProject(p);
	p->editors_.append(e);
	projects_.append(p);
	appendEditor(e);
	insertRubbish(e);
}

void AeWindow::insertRubbish(AeEditor *e) {
	e->insertPlainText(
"/* This is sample\n"
"C++ code */ \n"
"#include <iostream>\n"
"#include <cstdio>\n"
"#include <vector>\n"
"#define NEGATIVE(x) (-x)\n"
"\n"
"namespace s = std;\n"
"using namespace std;\n"
"\n"
"// This comment may span only this line\n"
"typedef unsigned int uint;\n"
"int static myfunc(uint parameter) {\n"
"	if (parameter == 0)\n"
"		fprintf(stdout, \"zero\\n\");\n"
"	cout << \"hello\\n\";\n"
"	return parameter - 1;\n"
"}\n\n"
"namespace ns {\n"
"	void nsFunc() {}\n"
"}\n\n"
"template<typename T>\n"
"struct CustomVector : public s::vector<T> {\n"
"	vector<T>* me() const { return this; }\n"
"};\n"
"\n"
"class MyClass {\n"
"public:\n"
"	enum Number { ZERO, ONE=9, TWO };\n"
"	static char staticField;\n"
"	int field;\n"
"	virtual Number vmethod();\n"
"	void method(Number n) const {\n"
"		int local = (int) NEGATIVE(n);\n"
"label:\n"
"		myfunc(local);\n"
"		staticMethod();\n"
"		undefinedMethod();\n"
"		ns::nsFunc();\n"
"	}\n"
"	static void staticMethod();\n"
"};\n"
"\n"
"void MyClass::staticMethod() {}\n"
"int main() {\n"
"	CustomVector<MyClass> v;\n"
"	vector<int>* p = v.me();\n"
"	v.at(0).vmethod();\n"
"}\n");
}

void AeWindow::appendEditor(AeEditor* e) {
	m_tabs->addTab(e, e->displayName());
	m_tabs->setCurrentWidget(e);
	e->setFocus();
}

bool AeWindow::closeEditor(int tabindex) {
	AeEditor *e = (AeEditor*) m_tabs->widget(tabindex);

	if(e->dirty()) {
		QMessageBox confirm;
		confirm.setIcon(QMessageBox::Question);
		confirm.setText("The document `" + e->displayName() + "' has been modified");
		confirm.setInformativeText("Do you want to save your changes?");
		confirm.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		QAbstractButton* diffButton = confirm.addButton("Show diff...", QMessageBox::ActionRole);

		int result = confirm.exec();
		while(confirm.clickedButton() == diffButton) {
			on_actionDiff_to_Saved_triggered();
			result = confirm.exec();
		}

		switch(result) {
		case QMessageBox::Save:
			if(!on_actionSave_triggered()) return false;
			break;
		case QMessageBox::Discard:
			break;
		case QMessageBox::Cancel:
			return false;
		default:
			ae_assert(false && "branch can't happen");
		}

	}

	AeProject* p = e->project();
	p->editors_.removeOne(e);
	if(p->editors_.length()==0) {
		projects_.removeOne(p);
		delete p;
	}
	delete m_tabs->widget(tabindex);
	m_tabs->removeTab(tabindex);
	return true;
}

bool AeWindow::closeEditors(int except) {
	int result = -1;
	for(int i=m_tabs->count() - 1; i>=0; --i) {
		AeEditor *e = (AeEditor*) m_tabs->widget(i);
		if(i == except)
			continue;
		if(e->dirty()) {
			switch(result) {
			case QMessageBox::YesToAll:
				if(!on_actionSave_triggered()) return false;
				break;
			case QMessageBox::NoToAll:
				break;
			default: {
				QMessageBox confirm;
				confirm.setIcon(QMessageBox::Question);
				confirm.setText("The document `" + e->displayName() + "' has been modified");
				confirm.setInformativeText("Do you want to save your changes?");
				confirm.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel);
				QAbstractButton* diffButton = confirm.addButton("Show diff...", QMessageBox::ActionRole);
				QAbstractButton* skipButton = confirm.addButton("Skip", QMessageBox::RejectRole);

				result = confirm.exec();
				while(confirm.clickedButton() == diffButton) {
					on_actionDiff_to_Saved_triggered();
					result = confirm.exec();
				}
				if(confirm.clickedButton() == skipButton) {
					continue;
				} else if(result == QMessageBox::Save || result == QMessageBox::YesToAll) {
					if(!on_actionSave_triggered()) return false;
				} else if(result == QMessageBox::Cancel)
					return false;
			}
			break;
			}
		}
		delete m_tabs->widget(i);
		m_tabs->removeTab(i);
	}
	return m_tabs->count()==0;
}

void AeWindow::handleDirtied(QWidget * w, bool dirty) {
	const AeEditor* e = (AeEditor*) w;
	ui->actionDiff_to_Saved->setEnabled(dirty && e->fileExists());
	ui->actionRevert_to_Saved->setEnabled(e->fileExists() && e->dirty());
	ui->actionSave->setEnabled(dirty);
	int index = m_tabs->indexOf(w);
	ae_info("Setting on index " << index);
	m_tabs->tabBar()->setTabTextColor(index, dirty? Qt::red : Qt::black);
}

void AeWindow::open(QString fileName) {
	ae_info("Opening " << fileName.toUtf8().constData() << "?");
	// first check to see if the file is already open. If so, just focus it.
	for(int i=m_tabs->count()-1; i>=0; --i) {
		AeEditor* e = (AeEditor*) m_tabs->widget(i);
		if(e->filePath() == fileName) {
			m_tabs->setCurrentWidget(e);
			e->focusWidget();
			return;
		}
	}
	// if we make it here, the file is not already open. Create a new editor.
	AeEditor * e = createEditor<AeCxxModel>();
	if(e->openFile(fileName)) {
		AeProject* p = AeProject::getProject(projects_, fileName);
		e->setProject(p);
		p->editors_.append(e);
		if(!projects_.contains(p)) projects_.append(p);
		appendEditor(e);
	} else {
		QMessageBox::warning(this, "Error", "Could not open " + fileName);
		delete e;
	}
}

void AeWindow::on_actionOpen_triggered() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "");
	if(!fileName.isEmpty()) {
		open(fileName);
	}
}

void AeWindow::closeEvent(QCloseEvent * e) {
	if(closeEditors(-1))
		e->accept();
	else
		e->ignore();
}

void AeWindow::on_actionExit_triggered() {
	close();
}

bool AeWindow::save(int tabindex) {
	AeEditor* e = (AeEditor*) m_tabs->widget(tabindex);
	if(!e->saveFile()) {
		QMessageBox::warning(this, "Error", "Could not save " + e->displayName());
		return false;
	}

	// if we're here, all went well
	m_tabs->tabBar()->setTabTextColor(tabindex, Qt::black);
	m_tabs->setTabText(tabindex, e->displayName());
	currentTabChanged(tabindex);
	return true;
}

bool AeWindow::save(int tabindex, QString location) {
	AeEditor* e = (AeEditor*) m_tabs->widget(tabindex);
	if(!e->saveFile(location)) {
		QMessageBox::warning(this, "Error", "Could not save to " + location);
		return false;
	}
	AeProject* p = AeProject::getProject(projects_, location);
	if(p != e->project()) {
		e->project()->editors_.removeOne(e);
		if(e->project()->editors_.length() == 0)
			projects_.removeOne(e->project());
		e->setProject(p);
		p->editors_.append(e);
		if(!projects_.contains(p))
			projects_.append(p);
	}
	// if we're here, all went well
	m_tabs->tabBar()->setTabTextColor(tabindex, Qt::black);
	currentTabChanged(tabindex);
	return true;
}

bool AeWindow::on_actionSave_triggered() {
	AeEditor* e = (AeEditor*) m_tabs->currentWidget();
	if(e->fileExists())
		return save(m_tabs->currentIndex());
	else
		return on_actionSave_As_triggered();
}

bool AeWindow::on_actionSave_As_triggered() {
	QString newFileName = QFileDialog::getSaveFileName(this);
	if(newFileName.isNull()) //cancelled
		return false;
	else // attempt to save
		return save(m_tabs->currentIndex(), newFileName);
}

void AeWindow::on_actionDiff_to_Saved_triggered()
{
	AeEditor* e = (AeEditor*) m_tabs->currentWidget();
	QTemporaryFile tmpfile(QDir::tempPath() + "/aristed.tmp." + e->displayName() + ".XXXXXX");
	if(!tmpfile.open()) {
		QMessageBox::warning(this, "Error", "Could not open " + tmpfile.fileName() + " for writing");
		return;
	}
	const QByteArray& data = e->toPlainText().toLocal8Bit();
	if(tmpfile.write(data) != data.size()) {
		QMessageBox::warning(this, "Error", "Could not write the correct number of bytes to " + tmpfile.fileName());
		return;
	}
	QStringList args;
	args << e->filePath() << tmpfile.fileName();
	if(int status = QProcess::execute("meld", args) != 0)
		QMessageBox::warning(this, "Error", "QProcess::execute returned " + QString::number(status));
}

void AeWindow::on_actionRevert_to_Saved_triggered() {
	AeEditor* e = (AeEditor*) m_tabs->currentWidget();
	// should not be possible to trigger this action if there is no file
	ae_assert(e->fileExists());

	QMessageBox confirm;
	confirm.setIcon(QMessageBox::Question);
	confirm.setText("Your changes to `" + e->displayName() + "' will be lost");
	confirm.setInformativeText("Are you sure you wish to proceed?");
	confirm.setStandardButtons(QMessageBox::Yes | QMessageBox::No);

	if(confirm.exec() == QMessageBox::Yes)
		e->openFile(e->filePath());
}

void AeWindow::on_actionClose_triggered() {
	closeEditor(m_tabs->currentIndex());
}

void AeWindow::on_actionClose_Others_triggered()
{
	closeEditors(m_tabs->currentIndex());
}

void AeWindow::on_actionShow_File_Manager_triggered()
{
	if(ui->fileTree->isHidden())
		ui->fileTree->show();
	else
		ui->fileTree->hide();
}

void AeWindow::on_fileView_activated(const QModelIndex &index)
{
	if(!dirModel_->isDir(index)) {
		QString file = dirModel_->filePath(index);
		open(file);
	}

}

void AeWindow::on_actionBuild_Run_triggered()
{
	AeEditor* e = (AeEditor*) m_tabs->currentWidget();
	if(e->project()->build(ui->consoleWindow, true))
		ui->console->show();
}

void AeWindow::on_actionOpen_Resource_triggered()
{
	AeEditor* e = (AeEditor*) m_tabs->currentWidget();
	AeFileSearchDialog* fsd = new AeFileSearchDialog(e->project());
	fsd->exec();
	ae_info("User selected " << fsd->fileToOpen());
	open(fsd->fileToOpen());
}
