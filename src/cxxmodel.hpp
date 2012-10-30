#ifndef CXXEDITOR_HPP
#define CXXEDITOR_HPP

#include "codemodel.hpp"
#include <QSyntaxHighlighter>
#include <QModelIndex>
#include "lockable.hpp"
#include "textstyle.hpp"
#include <QTextCursor>
#include <QKeyEvent>
#include <clang-c/Index.h>
// forward decls
class QCompleter;
//typedef void* CXIndex;
//typedef struct CXTranslationUnitImpl *CXTranslationUnit;

class CxxModel;
class Editor;
class Highlighter;
class CxxBackground : public QObject {
	Q_OBJECT
public:
	CxxBackground(CxxModel* e, void (CxxModel::*mfun)(char)) :
		QObject(),
		e(e),
		mfun(mfun)
	{}
signals:
	void complete(char);
public slots:
	void launch(char s);
private:
	CxxModel* e;
	void (CxxModel::*mfun)(char);
};
struct ColourScheme;
class CxxModel : public CodeModel {
	Q_OBJECT
public:
	CxxModel(Highlighter &highlighter, const ColourScheme* const& colours, QString filename);
	~CxxModel();
	int rowCount(const QModelIndex &) const;
	QVariant data(const QModelIndex &, int ) const;
	QString getTipAt(int row, int col);
void handleTextChanged(QTextDocument* document, int position, int, int);
protected:
bool keyPressEvent(QPlainTextEdit*,QKeyEvent *);
virtual QString completionPrefix() const { return mCompletionPrefix; }
	QThread* backgroundWorker;
	CXIndex index;
	QVector<QString> completionResults;
	Lockable<QTextCursor> mCompletionCursor;
	Lockable<CXTranslationUnit> tu;
	CXCursor mCursor;
	void setFileName(QString name);
	QString fileName_;
	void handleCursorMoved();
	void lexicalHighlight();
	void semanticHighlight();
	QString mCompletionPrefix;
	void prepareCompletions(QTextDocument *doc);
	void cursorPositionChanged(QTextDocument* doc, QTextCursor cur);
	//const TextStyle* getStyle(int blockNumber, int index);
	void reparseDocument(char s);
	void findCursorInfo(char s);
	char threadSentinel;
	Highlighter& highlighter;
	const ColourScheme * const& colours;
	Lockable<QByteArray> lastDocument;
	QMutex mutex;
	/*
	typedef QMap<int, HighlightStyleVector> HighlightStyleMap;
	typedef QMap<int, DiagStyleVector> DiagnosticStyleMap;
	Lockable<HighlightStyleMap> blockHighlightStyles;
	Lockable<DiagnosticStyleMap> blockDiagnosticStyles;*/
	StyleMap* newStyles_;

	struct Annotation {
		int start;
		int length;
		QString message;
	};
	Lockable< QMap<int, QVector<Annotation> > > codeAnnotations;

Editor* e;
CxxBackground semantics;
CxxBackground cursorInfo;
signals:
void positionInfo(QString);

protected slots:
	void reparseComplete(char);
	void cursorInfoFound(char);
};

#endif // CXXEDITOR_HPP
