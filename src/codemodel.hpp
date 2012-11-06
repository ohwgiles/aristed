#ifndef CODEMODEL_HPP
#define CODEMODEL_HPP

#include <QAbstractListModel>
#include <QTextCursor>

class TextStyle;
class QKeyEvent;
class QPlainTextEdit;

// The code model provides an interface used by the editor to
// generate and retrieve semantic information about the document
class AeCodeModel : public QAbstractListModel {
public:
	AeCodeModel(QObject* parent = 0);
	virtual ~AeCodeModel() {}

	// The completion prefix is the text string used to filter the
	// results returned by the QCompleter object owned by the editor
	virtual QString completionPrefix() const { return ""; }

	// a code model implement this to update its semantic tree
	virtual void handleTextChanged(QTextDocument*, int, int, int) {}

	// called by the editor when the user wants to show the completion options
	virtual void prepareCompletions(QTextDocument*) {}

	// a code model can implement this to prepare for a code jump,
	// or to update the user with pertinent information about the symbol
	virtual void cursorPositionChanged(QTextDocument* , QTextCursor ) {}

	// language-specific keybindings. return true if handled, false if not
	virtual bool keyPressEvent(QPlainTextEdit*,QKeyEvent*) { return false; }

	// return a tooltip for the given row,column
	virtual QString getTipAt(int, int) { return ""; }

	// language model may require the source code filename
	virtual void setFileName(QString) {}

	enum IndentType { INDENT_TAB, INDENT_SPACE };

	// Given a code line, determine whether tabs or spaces were used for indentation
	IndentType getIndentType(QString line) const;

	// Determine how many levels of indentation a line has
	int getIndentLevel(QString line, IndentType type) const;

	int spacesPerIndent() const { return 3; }

	// Return a string (of tabs or spaces)
	QString indentString(int level, IndentType type) const;
};

#endif // CODEMODEL_HPP
