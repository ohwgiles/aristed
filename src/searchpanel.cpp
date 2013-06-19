/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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
	lineEdit_->installEventFilter(this);
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

bool AeSearchPanel::eventFilter(QObject * o, QEvent * event) {
	if(o == lineEdit_ && event->type() == QEvent::KeyPress) {
		QKeyEvent* e = (QKeyEvent*) event;
		if(e->key() == Qt::Key_Return) {
			emit searchConfirmed(e->modifiers() & Qt::SHIFT);
			return true;
		}
	}
	return false;
}

void AeSearchPanel::paintEvent(QPaintEvent *e) {
	QPainter painter(this);

	painter.fillRect(e->rect(), colourScheme_->panelBackground());
	painter.setPen(Qt::DotLine);
	painter.drawLine(0, 0, width(), 0);


	QWidget::paintEvent(e);
}
