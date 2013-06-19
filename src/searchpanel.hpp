#ifndef SEARCHPANEL_HPP
#define SEARCHPANEL_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QWidget>

class AeEditor;
class QLineEdit;
struct ColourScheme;

class AeSearchPanel : public QWidget {
	Q_OBJECT
public:
	explicit AeSearchPanel(AeEditor *editor);
	void setColourScheme(const ColourScheme* c);

protected:
	//void keyPressEvent(QKeyEvent *);
	bool eventFilter(QObject *, QEvent *);
	void focusInEvent(QFocusEvent *);
	void paintEvent(QPaintEvent *);
signals:
	void searchConfirmed(bool);

private:
	QLineEdit* lineEdit_;
	const ColourScheme* colourScheme_;
};

#endif // SEARCHPANEL_HPP
