#ifndef LINENUMBERBAR_HPP
#define LINENUMBERBAR_HPP

#include <QWidget>
#include "editor.hpp"

class LineNumberBar : public QWidget
{
	Q_OBJECT
public:
	explicit LineNumberBar(Editor *editor) : QWidget(editor), editor(editor) {}

	QSize sizeHint() const {
		return QSize(editor->lineNumberBarWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) {
		editor->lineNumberBarPaintEvent(event);
	}

private:
	Editor *editor;
};

#endif // LINENUMBERBAR_HPP
