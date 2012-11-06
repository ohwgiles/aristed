#ifndef SEARCHPANEL_HPP
#define SEARCHPANEL_HPP

#include <QWidget>

class AeEditor;
class QLineEdit;

class AeSearchPanel : public QWidget {
	Q_OBJECT
public:
	explicit AeSearchPanel(AeEditor *editor);
	
protected:
	void keyPressEvent(QKeyEvent *);
	void focusInEvent(QFocusEvent *);

signals:
	void searchConfirmed(bool);

private:
	QLineEdit* lineEdit_;
	
};

#endif // SEARCHPANEL_HPP
