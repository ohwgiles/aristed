#ifndef CXXEDITOR_HPP
#define CXXEDITOR_HPP

#include "editor.hpp"
#include "clangfile.hpp"
#include <QAbstractItemModel>
#include <QSyntaxHighlighter>

class QCompleter;

class CxxEditor : public Editor {
	Q_OBJECT
public:
	CxxEditor(QWidget* parent = 0);
protected:
	virtual void keyPressEvent(QKeyEvent *e);
	void handleCursorMoved();
	void lexicalHighlight();
	void semanticHighlight();
	void highlightToken(int offset, int len, const QTextCharFormat& tcf);
	ClangFile m_clangfile;
	bool event(QEvent * e);
	QCompleter* mCompleter;
	QString mCompletionPrefix;
	void complete();

	void updateColour(QColor colour, unsigned offset, unsigned len);
protected slots:
	void completionRequested(QString);
	void handleTextChanged(int, int, int);
	void updateLexicalColours(bool markDirty=true);
	void updateSemantics(const char sentinel);
};

#endif // CXXEDITOR_HPP
