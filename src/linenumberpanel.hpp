
#ifndef LINENUMBERPANEL_HPP
#define LINENUMBERPANEL_HPP
#include <QWidget>
class Editor;
struct ColourScheme;
class LineNumberPanel : public QWidget {
public:
	LineNumberPanel(Editor *e);
	virtual ~LineNumberPanel() {}
	void setColourScheme(const ColourScheme* c) { colourScheme_ = c; }

protected:
	virtual void paintEvent(QPaintEvent *);
	virtual QSize sizeHint() const { return QSize(w,0); }
//	virtual QSize minimumSize() const;
	Editor* editor_;
	const ColourScheme* colourScheme_;
int w;
};

#endif // LINENUMBERPANEL_HPP

