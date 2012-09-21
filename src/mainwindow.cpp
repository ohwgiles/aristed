#include "mainwindow.hpp"
#include "ui_mainwindow.h"

#include "editor.hpp"
#include "cxxmodel.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include <QTabWidget>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QMessageBox>
#include <QPushButton>
#include <QProcess>
#include <QLabel>
MainWindow::MainWindow(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindow),
	mColourScheme(new colour::SolarizedLight())
{
	ui->setupUi(this);
	m_tabs = new TabWidget(this);
	QIcon icon(":icon.png");

	setWindowIcon(icon);

	this->setCentralWidget(m_tabs);
	connect(m_tabs, SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged(int)));

	cursor_position = new QLabel();
	ui->statusbar->addPermanentWidget(cursor_position);

	on_actionNew_triggered();
}

void MainWindow::currentTabChanged(int index) {
	ae_info("Changed tab to " << index);
	Editor& e = *m_editors[index];
	//disconnect(this, SLOT(handleDirtied(bool)));

	setWindowTitle(e.displayName() + " - aristed");
	ui->actionDiff_to_Saved->setEnabled(e.hasFileName() && e.dirty());
	ui->actionSave->setEnabled(e.dirty());
}

MainWindow::~MainWindow() {
	delete ui;
	delete m_tabs;
}

template<typename Model>
Editor* MainWindow::createEditor() {
	Editor* e = new Editor(this);
	e->setCxxModel();
	e->setColourScheme(mColourScheme);
	connect(e, SIGNAL(dirtied(bool)), this, SLOT(handleDirtied(bool)));
	//connect(e, SIGNAL(positionInfo(QString)), ui->statusbar, SLOT(showMessage(QString)));
	connect(e, SIGNAL(updateCursorPosition(QString)), cursor_position, SLOT(setText(QString)));

	return e;
}

void MainWindow::on_actionNew_triggered()
{
	Editor * e = createEditor<CxxModel>();
	appendEditor(e);
	insertRubbish(e);

//	open("/home/og/dev/clang-3.0.src/lib/Sema/SemaTemplateInstantiate.cpp");
}

void MainWindow::insertRubbish(Editor *e) {
	e->insertPlainText("/* This is sample C++ code*/ \n"
"#include <iostream>\n"
"#include <cstdio>\n"
"#include <vector>\n"
"#define MACRO(x) x\n"
"using namespace std;\n"
"// This comment may span only this line\n"
"typedef unsigned int uint;\n"
"int static myfunc(uint parameter) {\n"
"  if (parameter == 0) fprintf(stdout, \"zero\\n\");\n"
"  cout << \"hello\\n\";\n"
"  return parameter - 1;\n"
"}\n"
"namespace ns {\n"
" void doNothing() {}\n"
"}\n"
"template<typename T>\n"
"class CustomVector : public std::vector<T> {\n"
""
"};"
"class MyClass {\n"
"public:\n"
"  enum Number { ZERO, ONE, TWO };\n"
"  static char staticField;\n"
"  int field;\n"
"  virtual Number vmethod();\n"
"  void method(Number n) const {\n"
"    int local= (int)MACRO('\\0');\n"
"label: myfunc(local);\n"
"    vmethod();\n"
"    staticMethod();\n"
"    problem();\n"
"    ns::doNothing();\n"
"  }\n"
"  static void staticMethod();\n"
"};\n"
"\n"
"int main() {\n"
"    vector<MyClass> v;\n"

"}\n");
}

void MainWindow::appendEditor(Editor* e) {
	m_editors.push_back(e);
	m_tabs->addTab(e, e->displayName());
	m_tabs->setCurrentWidget(e);
}

bool MainWindow::closeEditor(int tabindex) {
	Editor *e = m_editors[tabindex];

	QMessageBox confirm;
	confirm.setIcon(QMessageBox::Question);
	confirm.setText("The document `" + e->displayName() + "' has been modified");
	confirm.setInformativeText("Do you want to save your changes?");
	confirm.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
	QAbstractButton* diffButton = confirm.addButton("Show diff...", QMessageBox::ActionRole);

	while(confirm.clickedButton() == diffButton) {
		on_actionDiff_to_Saved_triggered();
		confirm.exec();
	}

	switch(confirm.exec()) {
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

	delete m_editors[tabindex];
	m_editors.erase(m_editors.begin() + tabindex);
	m_tabs->removeTab(tabindex);
	return true;
}

void MainWindow::handleDirtied(bool dirty) {
	const Editor& e = *m_editors[m_tabs->currentIndex()];
	ui->actionDiff_to_Saved->setEnabled(dirty && e.hasFileName());
	ui->actionSave->setEnabled(dirty);
	ae_info("Setting on index " << m_tabs->currentIndex());
	m_tabs->tabBar()->setTabTextColor(m_tabs->currentIndex(), dirty? Qt::red : Qt::black);
}

void MainWindow::open(QString fileName) {
	Editor * e = createEditor<CxxModel>();
	if(e->openFile(fileName)) {
		appendEditor(e);
	} else {
		QMessageBox::warning(this, "Error", "Could not open " + fileName);
		delete e;
	}
}

void MainWindow::on_actionOpen_triggered() {
	QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "");
	if(!fileName.isEmpty()) {
		open(fileName);
	}
}

void MainWindow::on_actionExit_triggered() {
	close();
}

bool MainWindow::save(int tabindex) {
	Editor *e = m_editors[tabindex];
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

bool MainWindow::save(int tabindex, QString location) {
	Editor *e = m_editors[tabindex];
	if(!e->saveFile(location)) {
		QMessageBox::warning(this, "Error", "Could not save to " + location);
		return false;
	}
	// if we're here, all went well
	m_tabs->tabBar()->setTabTextColor(tabindex, Qt::black);
	currentTabChanged(tabindex);
	return true;
}

bool MainWindow::on_actionSave_triggered() {
	Editor& e = *m_editors[m_tabs->currentIndex()];
	if(e.hasFileName())
		return save(m_tabs->currentIndex());
	else
		return on_actionSave_As_triggered();
}

bool MainWindow::on_actionSave_As_triggered() {
	QString newFileName = QFileDialog::getSaveFileName(this);
	if(newFileName.isNull()) //cancelled
		return false;
	else // attempt to save
		return save(m_tabs->currentIndex(), newFileName);
}

void MainWindow::on_actionDiff_to_Saved_triggered()
{
	Editor* e = m_editors[m_tabs->currentIndex()];
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
	args << e->fileName() << tmpfile.fileName();
	if(int status = QProcess::execute("meld", args) != 0)
		QMessageBox::warning(this, "Error", "QProcess::execute returned " + QString::number(status));
}
