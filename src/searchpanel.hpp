#ifndef SEARCHPANEL_HPP
#define SEARCHPANEL_HPP

#include <QWidget>
class Editor;
class QLineEdit;
class SearchPanel : public QWidget
{
	Q_OBJECT
public:
	explicit SearchPanel(Editor *editor);
	
protected:
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent *);
signals:
	void searchConfirmed(bool);
public slots:
private:
	QLineEdit* le;
	
};

#endif // SEARCHPANEL_HPP
