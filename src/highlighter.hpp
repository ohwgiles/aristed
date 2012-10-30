#ifndef HIGHLIGHTER_HPP
#define HIGHLIGHTER_HPP

#include <QSyntaxHighlighter>
#include "textstyle.hpp"
class QPlainTextEdit;

#include <QMutex>


class Highlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	explicit Highlighter(QPlainTextEdit* e);
	void updateStyles(StyleMap* styleVector);

	// Fast modification - when we just move simple stuff around we can
	// do it quickly in the foreground thread. This is for basic moving
	// of colours and styles immediately when the user changes text
	void blockModified(QTextBlock block, int relativePos, int numInserted, int numRemoved);
protected:
	void highlightBlock(const QString &text);
private:
	StyleMap* styles_;
	QMutex mutex_;
};

#endif // HIGHLIGHTER_HPP
