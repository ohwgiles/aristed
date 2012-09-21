#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

class Editor;
class ColourScheme;
class QLabel;
namespace Ui {
class MainWindow;
}

// silly class to expose access to QTabBar
class TabWidget : public QTabWidget {
public:
	TabWidget(QWidget *p = 0) : QTabWidget(p){}
public:
	QTabBar *tabBar() const { return QTabWidget::tabBar(); }
};

class MainWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit MainWindow(QWidget *parent = 0);
	~MainWindow();
private:
	void appendEditor(Editor* e);
	void insertRubbish(Editor* e);

	template<typename E>
	Editor* createEditor();

	bool save(int tabindex);
	bool save(int tabindex, QString location);
	void open(QString file);
	bool closeEditor(int tabindex);

private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void currentTabChanged(int index);
	void handleDirtied(bool);
	void on_actionExit_triggered();
	bool on_actionSave_triggered();
	bool on_actionSave_As_triggered();
	void on_actionDiff_to_Saved_triggered();

private:
	TabWidget* m_tabs;
	Ui::MainWindow *ui;
	ColourScheme* mColourScheme;
	QVector<Editor *> m_editors;
	QLabel* cursor_position;
};

#endif // MAINWINDOW_HPP
