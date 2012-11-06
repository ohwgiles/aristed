#include "searchpanel.hpp"

#include <QLineEdit>
#include "editor.hpp"
#include <QVBoxLayout>
#include "log.hpp"
SearchPanel::SearchPanel(Editor *editor) :
	QWidget(editor)
{
	//setFixedHeight(40);

	QVBoxLayout* vbl = new QVBoxLayout();
	le = new QLineEdit();
	vbl->addWidget(le);
	this->setLayout(vbl);

	connect(le, SIGNAL(textChanged(QString)), editor, SLOT(searchString(QString)));
	connect(this, SIGNAL(searchConfirmed(bool)), editor, SLOT(moveToSearchResult(bool)));

}

void SearchPanel::focusInEvent(QFocusEvent *e) {
	le->setFocus();
	e->accept();
}



void SearchPanel::keyPressEvent(QKeyEvent *e) {
	if(e->key() == Qt::Key_Return) {
		ae_info("got enter");
		emit searchConfirmed(e->modifiers() & Qt::SHIFT);
		e->accept();
	}
}
