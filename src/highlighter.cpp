#include <QTextBlock>

#include "editor.hpp"
#include "log.hpp"
#include "codedecoration.hpp"

#include "highlighter.hpp"

AeHighlighter::AeHighlighter(QPlainTextEdit* e) :
	QSyntaxHighlighter(e),
	styles_(0)
{
	this->setDocument(e->document());
}

void AeHighlighter::blockModified(QTextBlock block, int relativePos, int added, int removed) {
	// We have to lock the mutex since a background thread may be
	// attempting to update the StyleVector pointer
	mutex_.lock();
	for(int i = 0, N = styles_->value(block.blockNumber()).size(); i != N; ++i) {
		AeCodeDecoration& ts = (*styles_)[block.blockNumber()][i];
		AeCodeDecoration::Extents extents = ts.extents();
		// if the highlight begins after our cursor position, advance
		// or subtract it by the amount of characters changed
		if(extents.start >= relativePos)
			extents.start += added - removed;
		// if the highlight ends before our cursor position,
		// leave it unchanged
		else if(extents.start + extents.length <= relativePos)
			{}
		// otherwise, we're in the middle of it. extend it. The normal
		// highlighting algorithm will correct this later.
		else
			extents.length += added - removed;
		ts.move(extents);
	}
	mutex_.unlock();

	// We've changed the colours so have to rehighlight this line
	rehighlightBlock(block);
}

void AeHighlighter::updateStyles(StyleMap *styleVector) {
	mutex_.lock();
	delete styles_;
	styles_ = new StyleMap(*styleVector);
	mutex_.unlock();
	rehighlight();
}

void AeHighlighter::highlightBlock(const QString &t) {
	mutex_.lock();

	const int blockNumber = currentBlock().blockNumber();
	if(blockNumber < 0) return;
	const StyleVector& v = styles_->value(blockNumber);
	for(int i=0, N=v.count(); i!=N; ++i) {
		const AeCodeDecoration& cd = v.at(i);
		QTextCharFormat f = cd.textCharFormat();
		AeCodeDecoration::Extents e = cd.extents();
		for(int j=0; j!=N; ++j) {
			if(j==i) continue;
			else if(e.start == v.at(j).extents().start && e.length == v.at(j).extents().length)
				f.merge(v.at(j).textCharFormat());
		}
		setFormat(cd.extents().start, cd.extents().length == -1 ? t.length() : cd.extents().length, f);
	}

	mutex_.unlock();
}
