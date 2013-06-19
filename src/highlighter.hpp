#ifndef HIGHLIGHTER_HPP
#define HIGHLIGHTER_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QMutex>
#include "codedecoration.hpp"

#include <QSyntaxHighlighter>

class QPlainTextEdit;

class AeHighlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	explicit AeHighlighter(QPlainTextEdit* e);
	void updateStyles(StyleMap* styleVector);

	// Fast modification - when we just move simple stuff around we can
	// do it quickly in the foreground thread. This is for basic moving
	// of colours and styles immediately when the user changes text
    void blockModified(QTextBlock block, unsigned column, unsigned numInserted, unsigned numRemoved);

protected:
	void highlightBlock(const QString &text);

private:
	StyleMap* styles_;
	QMutex mutex_;
};

#endif // HIGHLIGHTER_HPP
