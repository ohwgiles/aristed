#include <QLineEdit>
#include <QVBoxLayout>
#include <QPainter>
#include "editor.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include "searchpanel.hpp"

AeSearchPanel::AeSearchPanel(AeEditor *editor) :
	QWidget(editor)
{
	QVBoxLayout* vbl = new QVBoxLayout();
	lineEdit_ = new QLineEdit();
	lineEdit_->setFrame(false);

	vbl->addWidget(lineEdit_);
	vbl->setSpacing(0);
	vbl->setMargin(4);
	setLayout(vbl);

	connect(lineEdit_, SIGNAL(textChanged(QString)), editor, SLOT(searchString(QString)));
	connect(this, SIGNAL(searchConfirmed(bool)), editor, SLOT(moveToSearchResult(bool)));
}

void AeSearchPanel::focusInEvent(QFocusEvent *e) {
	lineEdit_->setFocus();
	e->accept();
}

void AeSearchPanel::setColourScheme(const ColourScheme* c) {
	colourScheme_ = c;
	QPalette p(palette());
	p.setColor(QPalette::Base, colourScheme_->background());
	p.setColor(QPalette::Text, colourScheme_->foreground());
	lineEdit_->setPalette(p);
}

void AeSearchPanel::keyPressEvent(QKeyEvent *e) {
	if(e->key() == Qt::Key_Return) {
		emit searchConfirmed(e->modifiers() & Qt::SHIFT);
		e->accept();
	}
	e->ignore();
}

void AeSearchPanel::paintEvent(QPaintEvent *e) {
	QPainter painter(this);

	painter.fillRect(e->rect(), colourScheme_->panelBackground());
	painter.setPen(Qt::DotLine);
	painter.drawLine(0, 0, width(), 0);


	QWidget::paintEvent(e);
}
