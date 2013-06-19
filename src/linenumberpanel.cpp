/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QPainter>
#include <QScrollBar>
#include <QLayout>
#include <QTextBlock>

#include "editor.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include "linenumberpanel.hpp"

AeLineNumberPanel::AeLineNumberPanel(AeEditor *e) : QWidget(e), editor_(e) {
	connect(e, SIGNAL(cursorPositionChanged()), this, SLOT(update()));
	connect(e->document(), SIGNAL(contentsChanged()), this, SLOT(update()));
	connect(e->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));

	const QFontMetrics fontMetrics = editor_->fontMetrics();
	setFixedWidth(fontMetrics.width("99") + 8);
}

void AeLineNumberPanel::paintEvent(QPaintEvent* e) {
	QPainter painter(this);

	painter.fillRect(e->rect(), colourScheme_->panelBackground());
	const QFontMetrics fontMetrics = editor_->fontMetrics();
	
	static const QChar wrappingIndicator(0x2937);

	const int lineSpacing = fontMetrics.lineSpacing();

	QTextDocument *document = editor_->document();
	const int currentLine = editor_->textCursor().blockNumber();

	const int pageBottom = editor_->viewport()->height();
	int lineNumber = (editor_->verticalScrollBar()->isVisible() ? editor_->verticalScrollBar()->value() : 0);
	int y = editor_->contentOffset().y() + lineSpacing - 3;
	
	painter.setPen(colourScheme_->foreground());
	QTextBlock block = document->findBlockByLineNumber(lineNumber);
	QString txt;
	while(block.isValid() && (y - lineSpacing) < pageBottom) {
		txt = QString::number(lineNumber + 1);
		if(lineNumber == currentLine) {
			painter.save();
			QFont f = painter.font();
			f.setWeight(QFont::Bold);
			painter.setFont(f);
		}

		painter.drawText(width() - fontMetrics.width(txt) - 4, y, txt);
		
		if(block.lineCount() > 1) {
			for(int i = 1; i < block.lineCount(); ++i) {
				painter.drawText(width() - fontMetrics.width(wrappingIndicator), y + i * lineSpacing, wrappingIndicator);
			}
		}
		
		if (lineNumber == currentLine)
			painter.restore();

		y += lineSpacing * block.lineCount();
		block = block.next();
		++lineNumber;
	}
	
	painter.setPen(Qt::DotLine);
	painter.drawLine(width()-1, 0, width()-1, pageBottom);
	int newWidth = fontMetrics.width(txt) + 8;
	if(newWidth != width()) {
		setFixedWidth(fontMetrics.width(txt) + 8);
		editor_->relayout();
	}
}

