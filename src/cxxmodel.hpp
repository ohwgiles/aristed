#ifndef CXXEDITOR_HPP
#define CXXEDITOR_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QSyntaxHighlighter>
#include <QModelIndex>
#include <QTextCursor>
#include <QKeyEvent>
#include <clang-c/Index.h>

#include "lockable.hpp"
#include "codedecoration.hpp"

#include "codemodel.hpp"

// forward decls
class QCompleter;
class AeCxxModel;
class AeEditor;
class AeHighlighter;
struct ColourScheme;

// Unfortunately I can't seem to get around requiring this class
// due to Qt's QObject model. This just wraps around a thread,
// calling back into the main CxxModel class to do the work
class CxxBackground : public QObject {
	Q_OBJECT
public:
	CxxBackground(AeCxxModel* e, void (AeCxxModel::*mfun)(char)) :
		QObject(),
		cxxModel_(e),
		memberCallback_(mfun)
	{}
signals:
	void complete(char);
public slots:
	void launch(char s);
private:
	AeCxxModel* cxxModel_;
	void (AeCxxModel::*memberCallback_)(char);
};

// The C++ Model. Interfaces into clang
class AeCxxModel : public AeCodeModel {
	Q_OBJECT
public:
	AeCxxModel(AeHighlighter &highlighter_, const ColourScheme* const& colourScheme_, QString filename);
	~AeCxxModel();

	// implemented from QAbstractItemList, for supplying completions
	int rowCount(const QModelIndex&) const;
	QVariant data(const QModelIndex&, int) const;

	// this model currently returns a diagnostic message, if any
	QString getTipAt(unsigned int row, unsigned int col);

signals:
	void positionInfo(QString);

private slots:
	void reparseComplete(char);
	void cursorInfoFound(char);

private:
	void handleTextChanged(QTextDocument* document, int position, int, int);
	bool keyPressEvent(QPlainTextEdit*,QKeyEvent *);
	QString completionPrefix() const { return completionPrefix_; }
	void setFileName(QString name);
	void handleCursorMoved();
	void lexicalHighlight();
	void semanticHighlight();
	void prepareCompletions(QTextDocument *doc);
	void cursorPositionChanged(QTextDocument* doc, QTextCursor cur);
	void reparseDocument(char s);
	void findCursorInfo(char s);

	struct Annotation {
        struct Rng {
            unsigned int start;
            unsigned int length;
        };
        QVector<Rng> ranges;
		QString message;
	};

	QThread* backgroundWorker_;
	CXIndex clangIndex_;
	QVector<QString> completionResults_;
	Lockable<QTextCursor> completionCursor_;
	Lockable<CXTranslationUnit> clangTranslationUnit_;
	CXCursor mCursor;
	QString fileName_;
	QString completionPrefix_;
	char threadSentinel;
	AeHighlighter& highlighter_;
	const ColourScheme * const& colourScheme_;
	Lockable<QByteArray> lastDocument_;
	QMutex mutex;
	StyleMap* newStyles_;
	Lockable< QMap<int, QVector<Annotation> > > codeAnnotations_;
	CxxBackground semanticsBuilder_;
	CxxBackground cursorInfoBuilder_;

};

#endif // CXXEDITOR_HPP
