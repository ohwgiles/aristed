
#ifndef LINENUMBERPANEL_HPP
#define LINENUMBERPANEL_HPP
#include <QWidget>
class Editor;
class LineNumberPanel : public QWidget {
public:
	LineNumberPanel(Editor *e);
	virtual ~LineNumberPanel() {}
protected:
	virtual void paintEvent(QPaintEvent *);
//	virtual QSize sizeHint() const;
//	virtual QSize minimumSize() const;
	Editor* editor_;
};

#endif // LINENUMBERPANEL_HPP

