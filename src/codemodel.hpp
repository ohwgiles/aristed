#ifndef CODEMODEL_HPP
#define CODEMODEL_HPP

#include <QAbstractListModel>
#include <QTextCursor>
class TextStyle;
class QKeyEvent;
class QPlainTextEdit;
class CodeModel : public QAbstractListModel
{
public:
	CodeModel(QObject* parent = 0);
	virtual ~CodeModel();
	virtual const TextStyle* getStyle(int blockNumber, int index) = 0;
	virtual QString completionPrefix() const { return ""; }
	virtual void handleTextChanged(QTextDocument* , int , int , int ) {}
	virtual void prepareCompletions(QTextDocument* ) {}
	virtual void cursorPositionChanged(QTextDocument* , QTextCursor ) {}
	virtual bool keyPressEvent(QPlainTextEdit*,QKeyEvent*) { return false; }
	virtual QString getTipAt(int , int ) { return ""; }
};

#endif // CODEMODEL_HPP
