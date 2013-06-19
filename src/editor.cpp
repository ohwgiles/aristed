/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QPainter>
#include <QPaintEvent>
#include <QTextBlock>
#include <QCompleter>
#include <QAbstractItemView>
#include <QFileInfo>
#include <QToolTip>

#include "colourscheme.hpp"
#include "log.hpp"
#include "codemodel.hpp"
#include "cxxmodel.hpp"
#include "linenumberpanel.hpp"
#include "searchpanel.hpp"
#include "diagnosticpanel.hpp"
#include "highlighter.hpp"

#include "editor.hpp"


AeEditor::AeEditor(QWidget *parent) :
	QPlainTextEdit(parent),
	project_(0),
	model_(0),
	fileExists_(false),
	dirty_(false)
{
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(highlightCurrentLine()));
	connect(this, SIGNAL(cursorPositionChanged()), this, SLOT(handleCursorMoved()));
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(handleTextChanged(int,int,int)));

	setMouseTracking(true);

	QTextOption opts;
	opts.setFlags(QTextOption::ShowTabsAndSpaces);
	document()->setDefaultTextOption(opts);

    setFont(QFont("Monaco", 9));
	setTabChangesFocus(false);
	setUndoRedoEnabled(true);
	setLineWrapMode(WidgetWidth);
	setTabStopWidth(fontMetrics().width('M')*3);
	setFrameStyle(0);

	highlighter_ = new AeHighlighter(this);

	completer_ = new QCompleter(this);
	completer_->setCompletionMode(QCompleter::PopupCompletion);
	completer_->setCaseSensitivity(Qt::CaseInsensitive);
	completer_->setWidget(this);
	connect(completer_, SIGNAL(activated(QString)), this, SLOT(completionChosen(QString)));

	lineNumberPanel_ = new AeLineNumberPanel(this);

	searchPanel_ = new AeSearchPanel(this);
	searchPanel_->hide();

    diagnosticPanel_ = new AeDiagnosticPanel(this);
    diagnosticPanel_->hide();

    relayout();
}

void AeEditor::searchString(QString str) {
	ae_info(str);
	lastSearchTerm_ = QRegExp(str);
	searchResults_.clear();
	QString doc = document()->toPlainText();
	int pos = 0;
	int len = 0;
	while((pos = doc.indexOf(lastSearchTerm_, pos+len)) != -1) {
		len = lastSearchTerm_.matchedLength();
		len = len > 0 ? len : 1;
		QTextEdit::ExtraSelection extra;
		extra.cursor = QTextCursor(document()->docHandle(), pos);
		extra.cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor, len);
		extra.format.setBackground(QBrush(colourScheme_->searchResult()));
		searchResults_.append(extra);
	}
	setExtraSelections(searchResults_);
}

void AeEditor::nextSearchResult(bool select) {
	QTextCursor start = textCursor();
	QTextCursor cur = document()->find(lastSearchTerm_, start);
	// if we haven't moved, the cursor was already at this result.
	// Search for the next one instead
	if(cur.selectionStart() == start.position()) {
		cur.movePosition(QTextCursor::NextCharacter);
		cur = document()->find(lastSearchTerm_, cur);
	}
	// if there is no result, try wrapped search (from beginning of doc)
	if(cur.isNull()) {
		cur = document()->find(lastSearchTerm_);
	}
	// if a result is found, go there
	if(!cur.isNull()) {
		start.setPosition(cur.selectionStart(), select? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
		setTextCursor(start);
	}
}

void AeEditor::prevSearchResult(bool select) {
	QTextCursor start = textCursor();
	QTextCursor cur = document()->find(lastSearchTerm_, start, QTextDocument::FindBackward);
	// if there is no result, try wrapped search (from end of doc)
	if(cur.isNull()) {
		cur = QTextCursor(document());
		cur.movePosition(QTextCursor::End);
		cur = document()->find(lastSearchTerm_, cur, QTextDocument::FindBackward);
	}
	// if a result is found, go there
	if(!cur.isNull()) {
		start.setPosition(cur.selectionStart(), select? QTextCursor::KeepAnchor : QTextCursor::MoveAnchor);
		setTextCursor(start);
	}
}

void AeEditor::moveToSearchResult(bool select) {
	setFocus();
	nextSearchResult(select);

}

AeEditor::~AeEditor() {
	highlighter_->setDocument(0);
	delete highlighter_;
	delete completer_;
	delete model_;
	delete searchPanel_;
	delete lineNumberPanel_;
}

void AeEditor::setCxxModel() {
	delete model_;
	model_ = new AeCxxModel(*highlighter_, colourScheme_, fileExists_ ? filePath_ : "untitled");
	completer_->setModel(this->model_);
}

void AeEditor::completionChosen(QString repl) {
	completer_->popup()->hide();
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, model_->completionPrefix().count());
	tc.removeSelectedText();
	insertPlainText(repl);
}

void AeEditor::setColourScheme(const ColourScheme* scheme) {
	colourScheme_ = scheme;
	QPalette p(palette());
	p.setColor(QPalette::Base, colourScheme_->background());
	p.setColor(QPalette::Text, colourScheme_->foreground());
	setPalette(p);
	highlightCurrentLine();
	lineNumberPanel_->setColourScheme(scheme);
	searchPanel_->setColourScheme(scheme);
    diagnosticPanel_->setColourScheme(scheme);

}
void AeEditor::handleTextChanged(int pos, int removed, int added) {
	setDirty(true);
	if(model_)
		model_->handleTextChanged(document(), pos, removed, added);
}

void AeEditor::showCompletions() {
	// Get the model to update its idea of the world
	model_->prepareCompletions(document());
	// Actually display the toolbox
	completer_->setCompletionPrefix(model_->completionPrefix());
	// Resetting the model appears to be necessary to get Qt to reproduce the
	// results. Otherwise, when the order of the results changes, the widget
	// will not reiterate them, and will return random results
	completer_->setModel(model_);
	QRect cr = cursorRect();
	QPoint p = viewport()->mapToParent(cr.topLeft());
	// TODO how wide should it be?
	completer_->complete(QRect(p.x(),p.y(), 140, cr.height()));

}

void AeEditor::handleCursorMoved() {
	// something cleans the search results. re-set them here
	setExtraSelections(searchResults_);


    // update current line:col status bar info
	QTextCursor cur = textCursor();
	QString loc = QString::number(cur.blockNumber()+1) + ":" + QString::number(cur.columnNumber());
	emit updateCursorPosition(loc);

	// Let the model update whatever it needs
	model_->cursorPositionChanged(document(), cur);

    // todo improve how this works
    QString noteHere = model_->getTipAt(cur.blockNumber(), cur.columnNumber());
    if(noteHere.isEmpty()) {
        if(diagnosticPanel_->isVisible()) {
            diagnosticPanel_->hide();
            relayout();
        }
    } else {
        bool wasVisible = diagnosticPanel_->isVisible();
        diagnosticPanel_->show(noteHere);
        if(!wasVisible) relayout();
    }

	if(completer_->popup()->isVisible())
		showCompletions();

}

void AeEditor::setDirty(bool b) {
	// only emit the signal if status changed
	if(dirty_ != b) {
		dirty_ = b;
		emit dirtied(this, b);
	}
}
void AeEditor::keyPressEvent(QKeyEvent *e) {
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

    if(e->key() == Qt::Key_Escape) {
        if(diagnosticPanel_->isVisible()) {
            diagnosticPanel_->hide();
            relayout();
            return;
        }

        if(searchPanel_->isVisible()) {
            searchPanel_->hide();
            searchResults_.clear();
            setExtraSelections(searchResults_);
            relayout();
            return;
        }
    }

	// allow the model to supply some key events
	if(model_->keyPressEvent(this, e))
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

	// searching
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Slash) {
		searchPanel_->show();
		relayout();
		searchPanel_->setFocus();
	}
	if(e->modifiers() & Qt::ALT && e->key() == Qt::Key_N)
		return nextSearchResult(e->modifiers() & Qt::SHIFT);
	if(e->modifiers() & Qt::ALT && e->key() == Qt::Key_P)
		return prevSearchResult(e->modifiers() & Qt::SHIFT);

	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Space) {
		showCompletions();
		return;
	}

	// If the completion window is available, enter/tab inserts the completion
	if(completer_->popup()->isVisible() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Tab)) {
		completionChosen(completer_->currentCompletion());
		return;
	}

	QPlainTextEdit::keyPressEvent(e);
}

bool AeEditor::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip) {
        QHelpEvent* he = (QHelpEvent*) e;

        //QPoint p = viewport()->mapFromGlobal(QCursor::pos());
        QPoint p = viewport()->mapFromGlobal(he->pos());
		QTextCursor tc = cursorForPosition(p);
		int col = tc.columnNumber();
		int row = tc.blockNumber();
		QString tip = model_->getTipAt(row, col);
		if(tip.isEmpty())
			QToolTip::hideText();
		else
			QToolTip::showText(QCursor::pos(), tip, this);
		return true;
	}
	return QPlainTextEdit::event(e);
}

bool AeEditor::openFile(QString fileName) {
	QFile f(fileName);
	if(f.open(QFile::ReadOnly)) {
		dirty_ = true;
		clear();
		insertPlainText(QString(f.readAll()));
		filePath_ = fileName;
		model_->setFileName(fileName);
		fileExists_ = true;
		setDirty(false);
		return true;
	}
	// failed!
	filePath_.clear();
	return false;
}

QString AeEditor::displayName() const {
	if(filePath_.isEmpty())
		return "<untitled>";
	QFileInfo fi(filePath_);
	return fi.fileName();
}

bool AeEditor::saveFile(QString fileName) {
	QFile f(fileName);
	if(!f.open(QFile::WriteOnly))
		return false;

	const QByteArray& data = toPlainText().toLocal8Bit();
	if(f.write(data) != data.size())
		return false;

	filePath_ = fileName;
	model_->setFileName(fileName);
	fileExists_ = true;
	setDirty(false);
	return true;
}
#include <QScrollBar>
void AeEditor::relayout() {
	QRect cr = contentsRect();
	int searchPanelHeight = searchPanel_->isVisible() ? searchPanel_->sizeHint().height() : 0;
    int diagnosticPanelHeight = diagnosticPanel_->isVisible() ? diagnosticPanel_->sizeHint().height() : 0;
    setViewportMargins(lineNumberPanel_->width(),0,0,searchPanelHeight + diagnosticPanelHeight);
    lineNumberPanel_->setGeometry(0,0,lineNumberPanel_->width(), cr.height() - searchPanelHeight - diagnosticPanelHeight);
    searchPanel_->setGeometry(0,cr.height()-searchPanelHeight,cr.width(),searchPanelHeight);
    diagnosticPanel_->setGeometry(0,cr.height()-diagnosticPanelHeight-searchPanelHeight,cr.width(),diagnosticPanelHeight);
    verticalScrollBar()->setGeometry(0,0,verticalScrollBar()->width(),cr.height()-searchPanelHeight-diagnosticPanelHeight);
}

void AeEditor::resizeEvent(QResizeEvent *e) {
    QPlainTextEdit::resizeEvent(e);
    relayout();
}

void AeEditor::highlightCurrentLine() {
	QList<QTextEdit::ExtraSelection> extraSelections;
	QTextEdit::ExtraSelection selection;

	selection.format.setBackground(colourScheme_->currentLineBg());
	selection.format.setProperty(QTextFormat::FullWidthSelection, true);
	selection.cursor = textCursor();
	selection.cursor.clearSelection();
	extraSelections.append(selection);

	setExtraSelections(extraSelections);
}
