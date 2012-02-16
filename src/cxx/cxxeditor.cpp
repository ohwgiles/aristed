#include "cxxeditor.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include <clang/AST/Decl.h>
#include <QPalette>
#include <QToolTip>
#include <QAbstractItemView>
#include <QCompleter>
CxxEditor::CxxEditor(QWidget *parent) :
	Editor(parent),
	m_clangfile()
{
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(handleTextChanged(int,int,int)));
	connect(&m_clangfile, SIGNAL(parsingComplete(char)), this, SLOT(updateSemantics(char)), Qt::QueuedConnection);
	connect(&m_clangfile, SIGNAL(lexingComplete()), this, SLOT(updateLexicalColours()), Qt::DirectConnection);
	setMouseTracking(true);
	mCompleter = new QCompleter(&m_clangfile, this);
	//m_clangfile.setCompletionWidget(mCompleter);
	m_clangfile.setParent(mCompleter);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	mCompleter->setWidget(this);
	connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(completionRequested(QString)));
}

void CxxEditor::completionRequested(QString repl) {
	ae_debug("CxxEditor::completionRequested");
	mCompleter->popup()->hide();
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, mCompletionPrefix.count());
	tc.removeSelectedText();
	insertPlainText(repl);

}

void CxxEditor::keyPressEvent(QKeyEvent *e) {
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Space) {
		ae_debug("Completing with prefix [" << mCompletionPrefix << "]");
		complete();
		return;
	}
	if(mCompleter->popup()->isVisible() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Tab)) {
		completionRequested(mCompleter->currentCompletion());
		return;
	}

	Editor::keyPressEvent(e);
	// todo cool things
}

bool CxxEditor::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip) {
		QPoint p = viewport()->mapFromGlobal(QCursor::pos());
		int cursor = cursorForPosition(p).position();
		m_clangfile.mSemanticMutex.lock();
		for(DiagVector::const_iterator it = m_clangfile.mDiagnosticInfo.constBegin(); it != m_clangfile.mDiagnosticInfo.constEnd(); ++it) {
			if(cursor >= it->offset && cursor <= it->offset + it->len) {
				QToolTip::showText(QCursor::pos(), it->content, this);
				break;
			} else
				QToolTip::hideText();
		}
		m_clangfile.mSemanticMutex.unlock();
		return true;
	} else
		return Editor::event(e);
}
void CxxEditor::complete() {
	mCompleter->setCompletionPrefix(mCompletionPrefix);
	QRect cr = cursorRect();
	QPoint p = viewport()->mapToParent(cr.topLeft());
	mCompleter->complete(QRect(p.x(),p.y(), 140, cr.height()));
}

void CxxEditor::handleCursorMoved() {
	QTextCursor cur = textCursor();
	//cur.movePosition(QTextCursor::PreviousCharacter);
	while(!cur.atBlockStart() && (
			document()->characterAt(cur.position()-1).isLetterOrNumber() ||
			document()->characterAt(cur.position()-1) == QChar('_'))) {
		ae_debug("Current cursor at " << document()->characterAt(cur.position()) << ", going backwards");
		cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	}
	mCompletionPrefix =  cur.selectedText();
	ae_debug("set completion prefix to " << mCompletionPrefix);
	m_clangfile.cursorMoved(cur);
	if(mCompleter->popup()->isVisible()) complete();
	// info("cursorPositionChanged");
	// m_cursor = clang_getCursor(tu, clang_getLocationForOffset(tu, file, textCursor().position());
	return;
}

void CxxEditor::highlightToken(int offset, int len, const QTextCharFormat& tcf) {
	QTextBlock bFrom = document()->findBlock(offset);
	QTextBlock bTo = document()->findBlock(offset + len);
	QTextLayout::FormatRange fmt;
	fmt.format = tcf;
	if(bFrom == bTo) {
		if(!bFrom.isValid())
			ae_fatal("request for block at offset " << offset << " resulted in invalid block. The doc is " << document()->characterCount() << " chars long");
		fmt.start = offset - bFrom.position();
		fmt.length = len;
		QList<QTextLayout::FormatRange> formats = bFrom.layout()->additionalFormats();
		formats.append(fmt);
		bFrom.layout()->setAdditionalFormats(formats);
	} else {
		fmt.start = offset - bFrom.position() - 1;
		fmt.length = bFrom.length() - (offset - bFrom.position());
		QList<QTextLayout::FormatRange> formats = bFrom.layout()->additionalFormats();
		formats.append(fmt);
		bFrom.layout()->setAdditionalFormats(formats);
		fmt.start = 0;
		QTextBlock b = bFrom.next();
		ae_assert(b.isValid());
		while(b != bTo) {
			fmt.length = b.length();
			formats = b.layout()->additionalFormats();
			formats.append(fmt);
			b.layout()->setAdditionalFormats(formats);
			b = b.next();
			ae_assert(b.isValid());
		}
		fmt.length = (bFrom.position() + len) - bTo.position();
		formats = b.layout()->additionalFormats();
		formats.append(fmt);
		b.layout()->setAdditionalFormats(formats);
	}
}

void CxxEditor::updateColour(QColor colour, unsigned offset, unsigned len) {
	QTextCharFormat tcf;
	tcf.setForeground(colour);
	highlightToken(offset, len, tcf);
}

void CxxEditor::handleTextChanged(int pos, int removed, int added) {
	QTextBlock b = document()->findBlock(pos);

	if(b == document()->findBlock(pos + added - removed)) {
		int p = pos - b.position(); // p is pos relative to the current block
		QList<QTextLayout::FormatRange> f = b.layout()->additionalFormats();
		for(QList<QTextLayout::FormatRange>::iterator it = f.begin(); it != f.end();) {
			if(it->start > p+1)
				it++->start += added - removed;
			else if(it->start + it->length < p)
				++it;
			else
				it++->length += added - removed;
		}
		b.layout()->setAdditionalFormats(f);
		document()->markContentsDirty(b.position(), b.length());
	}
	m_clangfile.update(document()->toPlainText());
}

void CxxEditor::updateLexicalColours(bool markDirty) {
	for(ColourVector::const_iterator it = m_clangfile.mLexicalColourInfo.constBegin(); it != m_clangfile.mLexicalColourInfo.constEnd(); ++it) {
		QTextCharFormat tcf;
		tcf.setForeground((mColourScheme->*it->fn)());
		highlightToken(it->offset, it->len, tcf);
	}
	if(markDirty)
	document()->markContentsDirty(0, document()->characterCount());
}

void CxxEditor::updateSemantics(const char sentinel) {
	if(!m_clangfile.semanticsValid(sentinel))
		return;

	m_clangfile.mSemanticMutex.lock();
	QTextBlock b = document()->firstBlock();
	while(b.isValid()) {
		b.layout()->clearAdditionalFormats();
		b = b.next();
	}
	updateLexicalColours(false);
	for(ColourVector::const_iterator it = m_clangfile.mSemanticColourInfo.constBegin(); it != m_clangfile.mSemanticColourInfo.constEnd(); ++it) {
		QTextCharFormat tcf;
		tcf.setForeground((mColourScheme->*it->fn)());
		highlightToken(it->offset, it->len, tcf);
	}
	for(DiagVector::const_iterator it = m_clangfile.mDiagnosticInfo.constBegin(); it != m_clangfile.mDiagnosticInfo.constEnd(); ++it) {
		QTextCharFormat f;
		f.setUnderlineColor(it->level == clang::DiagnosticsEngine::Error ? Qt::red : Qt::yellow);
		f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
		highlightToken(it->offset, it->len, f);
	}
	m_clangfile.mSemanticMutex.unlock();
	document()->markContentsDirty(0, document()->characterCount());
}
