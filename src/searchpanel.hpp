#ifndef SEARCHPANEL_HPP
#define SEARCHPANEL_HPP

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
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent *);
	void paintEvent(QPaintEvent *);
signals:
	void searchConfirmed(bool);

private:
	QLineEdit* lineEdit_;
	const ColourScheme* colourScheme_;
};

#endif // SEARCHPANEL_HPP
