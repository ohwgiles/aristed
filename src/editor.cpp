#include "editor.hpp"
#include "linenumberbar.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QFileInfo>

Editor::Editor(QWidget *parent) :
	QPlainTextEdit(parent),
	mLineNumberBar(new LineNumberBar(this)),
	mDirty(false),
	mHasFileName(false)
{
	connect(this, SIGNAL(blockCountChanged(int)), this, SLOT(updateLineNumberBarWidth(int)));
	connect(this, SIGNAL(updateRequest(QRect,int)), this, SLOT(updateLineNumberBar(QRect,int)));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(handleCursorMoved()));
	connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(handleDocModified(bool)));

	QTextOption opts;
	opts.setFlags(QTextOption::ShowTabsAndSpaces);
	setFont(QFont("Deja Vu Sans Mono", 12));
	setTabChangesFocus(false);
	setUndoRedoEnabled(true);
	setLineWrapMode(WidgetWidth);
	document()->setDefaultTextOption(opts);
	setTabStopWidth(fontMetrics().width('M')*3);
	updateLineNumberBarWidth(0);
}

void Editor::setColourScheme(ColourScheme* scheme) {
	mColourScheme = scheme;
	QPalette p(palette());
	p.setColor(QPalette::Base, mColourScheme->background());
	p.setColor(QPalette::Text, mColourScheme->foreground());
	setPalette(p);
	highlightCurrentLine();

}
void Editor::handleCursorMoved() {
	QTextCursor cur = textCursor();
	QString loc = QString::number(cur.blockNumber()+1) + ":" + QString::number(cur.columnNumber());
	emit updateCursorPosition(loc);

}

void Editor::setDirty(bool b) {
	if(mDirty != b) {
		mDirty = b;
		emit dirtied(b);
	}
}

void Editor::handleDocModified(bool) {
	setDirty(true);
}

bool Editor::openFile(QString fileName) {
	QFile f(fileName);
	if(f.open(QFile::ReadOnly)) {
		disconnect(SIGNAL(modificationChanged(bool)));
		insertPlainText(QString(f.readAll()));
		mFileName = fileName;
		setDirty(false);
		connect(this, SIGNAL(modificationChanged(bool)), this, SLOT(handleDocModified(bool)));
		mHasFileName = true;
		return true;
	}
	return false;
}

QString Editor::displayName() const {
	if(!mHasFileName)
		return "<Untitled>";
	QFileInfo fi(mFileName);
	return fi.fileName();
}

bool Editor::saveFile(QString fileName) {
	QFile f(fileName);
	if(!f.open(QFile::WriteOnly))
		return false;

	const QByteArray& data = toPlainText().toLocal8Bit();
	if(f.write(data) != data.size())
		return false;

	mFileName = fileName;
	mHasFileName = true;
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

void Editor::keyPressEvent(QKeyEvent *e) {
	//    // todo cool key bindings
	QPlainTextEdit::keyPressEvent(e);
	if(e->modifiers() & Qt::ControlModifier) {// && e->key() == Qt::Key_Space) {
		//handleTextChanged();
		//autoCompleteFromAPIs();
		return;
	}

	//    return QsciScintilla::keyPressEvent(e);
}

int Editor::lineNumberBarWidth() {
	int digits = 1;
	int max = qMax(1, document()->blockCount());
	while (max >= 10) {
		max /= 10;
		++digits;
	}
	return 8 + fontMetrics().width(QLatin1Char('9')) * digits;
}

void Editor::updateLineNumberBarWidth(int /* newBlockCount */) {
	setViewportMargins(lineNumberBarWidth(), 0, 0, 0);
}

void Editor::updateLineNumberBar(const QRect &rect, int dy) {
	if (dy)
		mLineNumberBar->scroll(0, dy);
	else
		mLineNumberBar->update(0, rect.y(), mLineNumberBar->width(), rect.height());
	if (rect.contains(viewport()->rect()))
		updateLineNumberBarWidth(0);
}

void Editor::resizeEvent(QResizeEvent *e) {
	QPlainTextEdit::resizeEvent(e);
	QRect cr = contentsRect();
	mLineNumberBar->setGeometry(QRect(cr.left(), cr.top(), lineNumberBarWidth(), cr.height()));
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

