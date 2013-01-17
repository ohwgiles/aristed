#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
class QFileSystemModel;
class AeEditor;
class QModelIndex;
struct ColourScheme;
class QLabel;
class AeProject;
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

class AeWindow : public QMainWindow {
	Q_OBJECT
public:
	explicit AeWindow(QList<AeProject*>& projects, QWidget *parent = 0);
	~AeWindow();
private:
	void appendEditor(AeEditor* e);
	void insertRubbish(AeEditor* e);

	template<typename E>
	AeEditor* createEditor();

	bool save(int tabindex);
	bool save(int tabindex, QString location);
	void open(QString file);
	bool closeEditor(int tabindex);
	bool closeEditors(int except);
	void closeEvent(QCloseEvent *);
private slots:
	void on_actionNew_triggered();
	void on_actionOpen_triggered();
	void currentTabChanged(int index);
	void handleDirtied(QWidget*, bool);
	void on_actionExit_triggered();
	bool on_actionSave_triggered();
	bool on_actionSave_As_triggered();
	void on_actionDiff_to_Saved_triggered();

	void on_actionRevert_to_Saved_triggered();

	void on_actionClose_triggered();

	void on_actionClose_Others_triggered();

	void on_actionShow_File_Manager_triggered();

	void on_fileView_activated(const QModelIndex &index);

	void on_actionBuild_Run_triggered();

private:
	TabWidget* m_tabs;
	Ui::MainWindow *ui;
	ColourScheme* mColourScheme;
	QFileSystemModel* dirModel_;
	//QVector<Editor *> m_editors;
	QLabel* cursor_position;

	QList<AeProject*>& projects_;
};

#endif // MAINWINDOW_HPP
