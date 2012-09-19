#ifndef CXXEDITOR_HPP
#define CXXEDITOR_HPP

#include "editor.hpp"
#include <QSyntaxHighlighter>
#include <QModelIndex>
#include "semanticthread.hpp"
#include "highlighter.hpp"
#include "lockable.hpp"
#include "textstyle.hpp"

// forward decls
class QCompleter;
typedef void* CXIndex;
typedef struct CXTranslationUnitImpl *CXTranslationUnit;
class CompletionModelProxy;


class CxxEditor : public Editor, private ThreadCaller {
	Q_OBJECT
public:
	CxxEditor(QWidget* parent = 0);
	// unfortunately cannot inherit qabstractlistmodel
	int rowCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &, int ) const;

protected:
	CompletionModelProxy* mCompletionModel;
	CXIndex index;
	QVector<QString> completionResults;
	QTextCursor mCompletionCursor;
	Lockable<CXTranslationUnit> tu;
	virtual void keyPressEvent(QKeyEvent *e);
	void handleCursorMoved();
	void lexicalHighlight();
	void semanticHighlight();
	bool event(QEvent * e);
	QCompleter* mCompleter;
	QString mCompletionPrefix;
	void complete();
	const TextStyle* getStyle(int blockNumber, int index);
	void doThreadWork(char s);
	char threadSentinel;
	Lockable<QByteArray> lastDocument;
	typedef QMap<int, HighlightStyleVector> HighlightStyleMap;
	typedef QMap<int, DiagStyleVector> DiagnosticStyleMap;
	Lockable<HighlightStyleMap> blockHighlightStyles;
	Lockable<DiagnosticStyleMap> blockDiagnosticStyles;
	Highlighter hlighter;

protected slots:
	void threadComplete(char);
	void completionChosen(QString);
	void handleTextChanged(int, int, int);
};

#endif // CXXEDITOR_HPP
