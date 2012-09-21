#ifndef CXXEDITOR_HPP
#define CXXEDITOR_HPP

#include "editor.hpp"
#include <QSyntaxHighlighter>
#include <QModelIndex>
#include "highlighter.hpp"
#include "lockable.hpp"
#include "textstyle.hpp"

#include <clang-c/Index.h>
// forward decls
class QCompleter;
//typedef void* CXIndex;
//typedef struct CXTranslationUnitImpl *CXTranslationUnit;
class CompletionModelProxy;

class CxxEditor;
class CxxBackground : public QObject {
	Q_OBJECT
public:
	CxxBackground(CxxEditor* e, void (CxxEditor::*mfun)(char)) :
		QObject(),
		e(e),
		mfun(mfun)
	{}
signals:
	void complete(char);
public slots:
	void launch(char s);
private:
	CxxEditor* e;
	void (CxxEditor::*mfun)(char);
};

class CxxEditor : public Editor {
	Q_OBJECT
public:
	CxxEditor(QWidget* parent = 0);
	~CxxEditor();
	// unfortunately cannot inherit qabstractlistmodel
	int rowCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &, int ) const;

protected:
	QThread* backgroundWorker;
	CompletionModelProxy* mCompletionModel;
	CXIndex index;
	QVector<QString> completionResults;
	Lockable<QTextCursor> mCompletionCursor;
	Lockable<CXTranslationUnit> tu;
	CXCursor mCursor;
	virtual void keyPressEvent(QKeyEvent *e);
	void handleCursorMoved();
	void lexicalHighlight();
	void semanticHighlight();
	bool event(QEvent * e);
	QCompleter* mCompleter;
	QString mCompletionPrefix;
	void complete();
	const TextStyle* getStyle(int blockNumber, int index);
	void reparseDocument(char s);
	void findCursorInfo(char s);
	char threadSentinel;
	Lockable<QByteArray> lastDocument;
	typedef QMap<int, HighlightStyleVector> HighlightStyleMap;
	typedef QMap<int, DiagStyleVector> DiagnosticStyleMap;
	Lockable<HighlightStyleMap> blockHighlightStyles;
	Lockable<DiagnosticStyleMap> blockDiagnosticStyles;
	Highlighter hlighter;

CxxBackground semantics;
CxxBackground cursorInfo;
signals:
void positionInfo(QString);
protected slots:
	void reparseComplete(char);
	void cursorInfoFound(char);
	void completionChosen(QString);
	void handleTextChanged(int, int, int);
};

#endif // CXXEDITOR_HPP
