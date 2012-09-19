#ifndef HIGHLIGHTER_HPP
#define HIGHLIGHTER_HPP

#include <QSyntaxHighlighter>

class Editor;




class Highlighter : public QSyntaxHighlighter {
	Q_OBJECT
public:
	explicit Highlighter(Editor* e);
protected:
	void highlightBlock(const QString &text);
private:
	Editor* e;
};

#endif // HIGHLIGHTER_HPP
