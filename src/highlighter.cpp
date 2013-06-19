/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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

void AeHighlighter::blockModified(QTextBlock block, unsigned column, unsigned added, unsigned removed) {
	// We have to lock the mutex since a background thread may be
	// attempting to update the StyleVector pointer
	mutex_.lock();
	for(int i = 0, N = styles_->value(block.blockNumber()).size(); i != N; ++i) {
		AeCodeDecoration& ts = (*styles_)[block.blockNumber()][i];
		AeCodeDecoration::Extents extents = ts.extents();
		// if the highlight begins after our cursor position, advance
		// or subtract it by the amount of characters changed
        if(extents.start >= column)
			extents.start += added - removed;
		// if the highlight ends before our cursor position,
		// leave it unchanged
        else if(extents.start + extents.length <= column)
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

void AeHighlighter::highlightBlock(const QString & t) {
	mutex_.lock();

	const int blockNumber = currentBlock().blockNumber();
	if(blockNumber < 0) return;
	const StyleVector& v = styles_->value(blockNumber);
    (void)t;
/*
	for(int i=0, N=v.count(); i!=N; ++i) {
		const AeCodeDecoration& cd = v.at(i);
		QTextCharFormat f = cd.textCharFormat();
		AeCodeDecoration::Extents e = cd.extents();
		for(int j=0; j!=N; ++j) {
			if(j==i) continue;
			else if(e.start == v.at(j).extents().start && e.length == v.at(j).extents().length)
				f.merge(v.at(j).textCharFormat());
		}
        if(cd.extents().length == -1) {
            ae_info("extents -1 at " << t);
        }
		setFormat(cd.extents().start, cd.extents().length == -1 ? t.length() : cd.extents().length, f);
    }*/

    unsigned from=0, to;
    bool done;
    do {
        done= true;
        to = currentBlock().length();
        QTextCharFormat f;
        for(int i=0;i<v.count();++i) {
            const AeCodeDecoration&cd =v.at(i);
            AeCodeDecoration::Extents e = cd.extents();
            if(e.start+e.length > (unsigned)currentBlock().length()) continue;
            if(from < e.start) {
                done = false;
                if(to > e.start) to = e.start;
            } else if(from < e.start + e.length) {
                done = false;
                if(to > e.start + e.length) to = e.start + e.length;
                f.merge(cd.textCharFormat());
            }
        }
        setFormat(from, to - from, f);
        from = to;
    } while(!done);

	mutex_.unlock();
}
