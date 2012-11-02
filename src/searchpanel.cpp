#include "searchpanel.hpp"

#include <QLineEdit>
#include "editor.hpp"
#include <QVBoxLayout>
SearchPanel::SearchPanel(Editor *editor) :
	QWidget(editor)
{
	//setFixedHeight(40);

	QVBoxLayout* vbl = new QVBoxLayout();
	QLineEdit* le = new QLineEdit();
	vbl->addWidget(le);
	this->setLayout(vbl);

	connect(le, SIGNAL(textChanged(QString)), editor, SLOT(searchString(QString)));
}
void SearchPanel::keyPressEvent(QKeyEvent *e) {

	e->accept();
}
