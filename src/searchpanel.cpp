#include <QLineEdit>
#include <QVBoxLayout>
#include "editor.hpp"
#include "log.hpp"

#include "searchpanel.hpp"

AeSearchPanel::AeSearchPanel(AeEditor *editor) :
	QWidget(editor)
{
	QVBoxLayout* vbl = new QVBoxLayout();
	lineEdit_ = new QLineEdit();
	vbl->addWidget(lineEdit_);
	this->setLayout(vbl);

	connect(lineEdit_, SIGNAL(textChanged(QString)), editor, SLOT(searchString(QString)));
	connect(this, SIGNAL(searchConfirmed(bool)), editor, SLOT(moveToSearchResult(bool)));
}

void AeSearchPanel::focusInEvent(QFocusEvent *e) {
	lineEdit_->setFocus();
	e->accept();
}

void AeSearchPanel::keyPressEvent(QKeyEvent *e) {
	if(e->key() == Qt::Key_Return) {
		emit searchConfirmed(e->modifiers() & Qt::SHIFT);
		e->accept();
	}
}

