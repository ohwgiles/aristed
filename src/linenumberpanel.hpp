
#ifndef LINENUMBERPANEL_HPP
#define LINENUMBERPANEL_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QWidget>

class AeEditor;
struct ColourScheme;

class AeLineNumberPanel : public QWidget {
public:
	AeLineNumberPanel(AeEditor *e);
	virtual ~AeLineNumberPanel() {}

	void setColourScheme(const ColourScheme* c) { colourScheme_ = c; }

private:
	virtual void paintEvent(QPaintEvent *);

	AeEditor* editor_;
	const ColourScheme* colourScheme_;
};

#endif // LINENUMBERPANEL_HPP

