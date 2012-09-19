#include "highlighter.hpp"
#include "editor.hpp"
#include "log.hpp"

#include "textstyle.hpp"
Highlighter::Highlighter(Editor *e) :
	QSyntaxHighlighter(e), e(e)
{
	this->setDocument(e->document());
}

void Highlighter::highlightBlock(const QString &t) {
	const int blockNumber = currentBlock().blockNumber();
	if(blockNumber < 0) return;
	int i = 0;
	const TextStyle* ts;
	while(0 != (ts = e->getStyle(blockNumber, i++))) {
		int l = ts->len;
		int s = ts->start;
		QTextCharFormat tcf = ts->toTcf();
		setFormat(s, l == -1 ? t.length() : l, tcf);
		//setFormat(ts->start, ts->len == -1 ? t.length() : ts->len, ts->toTcf());
	}
}
