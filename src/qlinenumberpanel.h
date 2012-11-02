
#ifndef LINENUMBERPANEL_HPP
#define LINENUMBERPANEL_HPP

#include "qpanel.h"

class LineNumberPanel : public Panel {
public:
	LineNumberPanel(Editor *e);
	virtual ~LineNumberPanel() {}
protected:
	virtual void paintEvent(QPaintEvent *);
//	virtual QSize sizeHint() const;
//	virtual QSize minimumSize() const;

};

#endif // LINENUMBERPANEL_HPP

