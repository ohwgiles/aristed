#include "linenumberpanel.hpp"
#include <QPainter>
#include <QScrollBar>
#include "editor.hpp"
#include "log.hpp"

LineNumberPanel::LineNumberPanel(Editor *e) : QWidget(e), editor_(e) {
	setFixedWidth(20);
	connect(e, SIGNAL(cursorPositionChanged()), this, SLOT(update()));
	connect(e->document(), SIGNAL(contentsChanged()), this, SLOT(update()));
	connect(e->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(update()));
}

//QSize LineNumberPanel::sizeHint() const {
//	QSize s = size();
//	s.setHeight(editor_->sizeHint().height());
//	return s;
//}
//QSize LineNumberPanel::minimumSize() const {
//	return QSize(width(),editor_->height());
//}


void LineNumberPanel::paintEvent(QPaintEvent* ) {
	//QWidget::paintEvent(event);
	QPainter painter(this);
	const QFontMetrics fontMetrics = editor_->fontMetrics();
	
	static const QChar wrappingIndicator(0x2937);

	const int lineSpacing = fontMetrics.lineSpacing();

	QTextDocument *document = editor_->document();
	const int currentLine = editor_->textCursor().blockNumber();
	
	const int pageBottom = editor_->viewport()->height();
	int lineNumber = (editor_->verticalScrollBar()->isVisible() ? editor_->verticalScrollBar()->value() : 0);
	int y = lineSpacing + 2;
	
	QTextBlock block = document->findBlockByLineNumber(lineNumber);
	QString txt;
	while(block.isValid()){// && (y - lineSpacing) < pageBottom) {
		txt = QString::number(lineNumber + 1);
		if(lineNumber == currentLine) {
			painter.save();
			QFont f = painter.font();
			f.setWeight(QFont::Bold);
			painter.setFont(f);
		}

		painter.drawText(width() - fontMetrics.width(txt) - 2, y, txt);
		
		if(block.lineCount() > 1) {
			for(int i = 1; i < block.lineCount(); ++i) {
				painter.drawText(width() - fontMetrics.width(wrappingIndicator) - 1, y + i * lineSpacing, wrappingIndicator);
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
	
	setFixedWidth(fontMetrics.width(txt) + 5);
}

