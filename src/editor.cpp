#include "editor.hpp"
#include "linenumberbar.hpp"
#include "colourscheme.hpp"
#include "log.hpp"
#include "codemodel.hpp"
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QCompleter>
#include <QAbstractItemView>
#include <QFileInfo>
#include "cxxmodel.hpp"
#include "linenumberpanel.hpp"
#include <QToolTip>
#include "highlighter.hpp"
#include "searchpanel.hpp"
Editor::Editor(QWidget *parent) :
	QPlainTextEdit(parent),
	fileExists_(false),
	//mLineNumberBar(new LineNumberBar(this)),
	mDirty(false),
	model(0),
	hlighter(this)
{
//	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberBarWidth(int)));
//	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberBar(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(handleCursorMoved()));
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(handleTextChanged(int,int,int)));

	setMouseTracking(true);

	QTextOption opts;
	opts.setFlags(QTextOption::ShowTabsAndSpaces);
	setFont(QFont("Deja Vu Sans Mono", 9));
	setTabChangesFocus(false);
	setUndoRedoEnabled(true);
	setLineWrapMode(WidgetWidth);
	document()->setDefaultTextOption(opts);
	setTabStopWidth(fontMetrics().width('M')*3);
	updateLineNumberBarWidth(0);
setFrameStyle(0);
	mCompleter = new QCompleter(this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	mCompleter->setWidget(this);
	connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(completionChosen(QString)));
	//EditorLayout* p = new EditorLayout(this);

	lineNumberPanel_ = new LineNumberPanel(this);
	//(void) lnp;
	//lnp->attach(this);
	//p->addWidget(lnp);
	//this->setLayout(p);
	searchPanel_ = new SearchPanel(this);
	searchPanel_->hide();
	relayout();

}

void Editor::searchString(QString str) {
	ae_info(str << " : " << find(str));
	lastSearchTerm_ = QRegExp(str);
	// unfortunately calling find() moves the cursor
	QTextCursor originalCursor = textCursor();
	moveCursor(QTextCursor::Start);
	QTextCursor cur(document()->docHandle(), 0);
	searchResults_.clear();
	while(!(cur = document()->find(lastSearchTerm_, cur)).isNull()) {
		QTextEdit::ExtraSelection extra;
		extra.cursor = cur;
		extra.format.setBackground(QBrush(mColourScheme->searchResult()));
		//extra.format.
		searchResults_.append(extra);
	}
	setExtraSelections(searchResults_);
	//setTextCursor(originalCursor);
}
void Editor::moveToSearchResult(bool select) {
	QTextCursor cur = textCursor();
	(void) select;
	if(!(cur = document()->find(lastSearchTerm_, cur)).isNull()) {
		if(!select)
			cur.setPosition(cur.selectionStart());
		setTextCursor(cur);
		setFocus();
	}
}

Editor::~Editor() {
	delete model;
	model = 0;
}

//const TextStyle* Editor::getStyle(int blockNumber, int index) { return model->getStyle(blockNumber, index); }

void Editor::setCxxModel() {
	delete model;
	model = new CxxModel(hlighter, mColourScheme, fileExists_ ? filePath_ : "untitled");
	this->model = model;
	mCompleter->setModel(this->model);
}

void Editor::completionChosen(QString repl) {
	mCompleter->popup()->hide();
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, model->completionPrefix().count());
	tc.removeSelectedText();
	insertPlainText(repl);
}

void Editor::setColourScheme(const ColourScheme* scheme) {
	mColourScheme = scheme;
	QPalette p(palette());
	p.setColor(QPalette::Base, mColourScheme->background());
	p.setColor(QPalette::Text, mColourScheme->foreground());
	setPalette(p);
	highlightCurrentLine();
	lineNumberPanel_->setColourScheme(scheme);

}
void Editor::handleTextChanged(int pos, int removed, int added) {
	ae_info("handleTextChanged");
	setDirty(true);

	if(model)
	model->handleTextChanged(document(), pos, removed, added);
}

void Editor::showCompletions() {
	// Get the model to update its idea of the world
	model->prepareCompletions(document());
	// Actually display the toolbox
	mCompleter->setCompletionPrefix(model->completionPrefix());
	// Resetting the model appears to be necessary to get Qt to reproduce the
	// results. Otherwise, when the order of the results changes, the widget
	// will not reiterate them, and will return random results
	mCompleter->setModel(model);
	QRect cr = cursorRect();
	QPoint p = viewport()->mapToParent(cr.topLeft());
	// TODO how wide should it be?
	mCompleter->complete(QRect(p.x(),p.y(), 140, cr.height()));

}

void Editor::handleCursorMoved() {
	// something cleans the search results. re-set them here
	setExtraSelections(searchResults_);

	QTextCursor cur = textCursor();
	QString loc = QString::number(cur.blockNumber()+1) + ":" + QString::number(cur.columnNumber());
	emit updateCursorPosition(loc);

	// Let the model update whatever it needs
	model->cursorPositionChanged(document(), cur);

	if(mCompleter->popup()->isVisible())
		showCompletions();

}

void Editor::setDirty(bool b) {
	// only emit the signal if status changed
	if(mDirty != b) {
		mDirty = b;
		emit dirtied(this, b);
	}
}
void Editor::keyPressEvent(QKeyEvent *e) {
	QTextCursor cur = textCursor();
	QTextCursor::MoveMode moveMode = e->modifiers() & Qt::SHIFT ? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor;

	if(navMode_ == FIND_UNTIL) {
		if(e->text().isEmpty()) return;
		navMode_ = NORMAL;
		while(!cur.atBlockEnd()) {
			if(document()->characterAt(cur.position()) == e->text().at(0))
				return setTextCursor(cur);
			cur.movePosition(QTextCursor::Right, lastMoveMode_);
		}
		return;
	}

	if(e->key() == Qt::Key_Escape && searchPanel_->isVisible()) {
		searchPanel_->hide();
		searchResults_.clear();
		setExtraSelections(searchResults_);
		relayout();
		return;
	}

	// allow the model to supply some key events
	if(model->keyPressEvent(this, e))
		return;

	// movement
	if(e->modifiers() & Qt::ALT) {
		switch(e->key()) {
		case Qt::Key_J: cur.movePosition(QTextCursor::Down, moveMode); break;
		case Qt::Key_K: cur.movePosition(QTextCursor::Up, moveMode); break;
		case Qt::Key_H: cur.movePosition(QTextCursor::Left, moveMode); break;
		case Qt::Key_L: cur.movePosition(QTextCursor::Right, moveMode); break;
		case Qt::Key_B: cur.movePosition(QTextCursor::WordLeft, moveMode); break;
		case Qt::Key_W: cur.movePosition(QTextCursor::WordRight, moveMode); break;
		default:
			goto alt_no_movement;
		}
		setTextCursor(cur);
		return;
		alt_no_movement: (void) 0;
	}
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_A)
		return cur.movePosition(QTextCursor::StartOfLine, moveMode), setTextCursor(cur);
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_E)
		return cur.movePosition(QTextCursor::EndOfLine, moveMode), setTextCursor(cur);
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_T)
		return (void)(navMode_ = FIND_UNTIL, lastMoveMode_ = moveMode);



	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Slash) {
		searchPanel_->show();
		relayout();
		searchPanel_->setFocus();
	}

	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Space) {
		showCompletions();
		return;
	}

	// If the completion window is available, enter/tab inserts the completion
	if(mCompleter->popup()->isVisible() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Tab)) {
		completionChosen(mCompleter->currentCompletion());
		return;
	}

	QPlainTextEdit::keyPressEvent(e);
}

bool Editor::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip) {
		QPoint p = viewport()->mapFromGlobal(QCursor::pos());
		QTextCursor tc = cursorForPosition(p);
		int col = tc.columnNumber();
		int row = tc.blockNumber();
		QString tip = model->getTipAt(row, col);
		if(tip.isEmpty())
			QToolTip::hideText();
		else
			QToolTip::showText(QCursor::pos(), tip, this);
		return true;
	}
	return QPlainTextEdit::event(e);
}

bool Editor::openFile(QString fileName) {
	QFile f(fileName);
	if(f.open(QFile::ReadOnly)) {
		//disconnect(SIGNAL(modificationChanged(bool)));
		mDirty = true;
		clear();
		insertPlainText(QString(f.readAll()));
		filePath_ = fileName;
		model->setFileName(fileName);
		fileExists_ = true;
		setDirty(false);
		//connect(document(), SIGNAL(modificationChanged(bool)), this, SLOT(handleDocModified(bool)));
		return true;
	}
	// failed!
	filePath_.clear();
	return false;
}

void Editor::handleDocModified(bool) {
//	ae_info("doc modified");
//	setDirty(true);

}

QString Editor::displayName() const {
	if(filePath_.isEmpty())
		return "<Untitled>";
	QFileInfo fi(filePath_);
	return fi.fileName();
}

bool Editor::saveFile(QString fileName) {
	QFile f(fileName);
	if(!f.open(QFile::WriteOnly))
		return false;

	const QByteArray& data = toPlainText().toLocal8Bit();
	if(f.write(data) != data.size())
		return false;

	filePath_ = fileName;
	model->setFileName(fileName);
	fileExists_ = true;
	setDirty(false);
	return true;
}

QTextCursor Editor::wordUnderCursor() const
{
	static QString eow(" \r\n\t~!@#$%^&*()_+{}|:\"<>?,./;'[]\\-=");
	//    this->getWord()
	QTextCursor tc = textCursor();
	QChar::Category cat;
	QChar c;
	do {
		tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
		c = tc.selectedText()[0];
		cat = c.category();
	} while(!tc.atStart() && cat != QChar::Punctuation_Connector && !c.isPunct() && !c.isSpace() && !c.isSymbol() && cat != QChar::NoCategory && c.isPrint());
	if(!tc.atStart())
		tc.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);
	return tc;
}

int Editor::lineNumberBarWidth() {
	int digits = 1;
	int max = qMax(1, document()->blockCount());
	// == floor(log10(max))
	while (max >= 10) {
		max /= 10;
		++digits;
	}
	return 8 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void Editor::updateLineNumberBarWidth(int /* newBlockCount */) {
//	setViewportMargins(lineNumberBarWidth(), 0, 0, 0);
}

void Editor::updateLineNumberBar(const QRect &rect, int dy) {
	if (dy)
		mLineNumberBar->scroll(0, dy);
	else
		mLineNumberBar->update(0, rect.y(), mLineNumberBar->width(), rect.height());
	if (rect.contains(viewport()->rect()))
		updateLineNumberBarWidth(0);
}

void Editor::relayout() {
	QRect cr = contentsRect();
	int searchPanelHeight = searchPanel_->isVisible() ? searchPanel_->sizeHint().height() : 0;
	setViewportMargins(lineNumberPanel_->width(),0,0,searchPanelHeight);
	lineNumberPanel_->setGeometry(0,0,lineNumberPanel_->width(), cr.height());
	searchPanel_->setGeometry(0,cr.height()-searchPanelHeight,cr.width(),searchPanelHeight);

}

void Editor::resizeEvent(QResizeEvent *e) {
	QPlainTextEdit::resizeEvent(e);
relayout();
//	QRect cr = contentsRect();
//	mLineNumberBar->setGeometry(QRect(cr.left(), cr.top(), lineNumberBarWidth(), cr.height()));
}

void Editor::highlightCurrentLine() {
	QList<QTextEdit::ExtraSelection> extraSelections;
	QTextEdit::ExtraSelection selection;

	selection.format.setBackground(mColourScheme->currentLineBg());
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = textCursor();
	selection.cursor.clearSelection();
	extraSelections.append(selection);

	setExtraSelections(extraSelections);
}

void Editor::lineNumberBarPaintEvent(QPaintEvent *event) {
	QPainter painter(mLineNumberBar);
	painter.fillRect(event->rect(), mColourScheme->lineNumberBackground());

	QTextBlock block = firstVisibleBlock();
	int blockNumber = block.blockNumber();
	int top = (int) blockBoundingGeometry(block).translated(contentOffset()).top();
	int bottom = top + (int) blockBoundingRect(block).height();
	while (block.isValid() && top <= event->rect().bottom()) {
		if (block.isVisible() && bottom >= event->rect().top()) {
			QString number = QString::number(blockNumber + 1);
			QFont f = font();
			if(block == textCursor().block())
				f.setBold(true);
			painter.setFont(f);
			painter.setPen(mColourScheme->foreground());
			painter.drawText(0, top, mLineNumberBar->width()-4, fontMetrics().height(), Qt::AlignRight, number);
		}

		block = block.next();
		top = bottom;
		bottom = top + (int) blockBoundingRect(block).height();
		++blockNumber;
	}
}

