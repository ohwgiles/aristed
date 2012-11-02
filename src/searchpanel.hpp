#ifndef SEARCHPANEL_HPP
#define SEARCHPANEL_HPP

#include <QWidget>
class Editor;

class SearchPanel : public QWidget
{
	Q_OBJECT
public:
	explicit SearchPanel(Editor *editor);
	
protected:
	void keyPressEvent(QKeyEvent *);
signals:
	
public slots:
	
};

#endif // SEARCHPANEL_HPP
