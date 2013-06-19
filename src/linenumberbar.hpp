#ifndef LINENUMBERBAR_HPP
#define LINENUMBERBAR_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QWidget>
#include "editor.hpp"

class LineNumberBar : public QWidget
{
	Q_OBJECT
public:
	explicit LineNumberBar(AeEditor *editor) : QWidget(editor), editor(editor) {}

	QSize sizeHint() const {
		return QSize(editor->lineNumberBarWidth(), 0);
	}

protected:
	void paintEvent(QPaintEvent *event) {
		editor->lineNumberBarPaintEvent(event);
	}

private:
	AeEditor *editor;
};

#endif // LINENUMBERBAR_HPP
