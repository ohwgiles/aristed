#ifndef CLANGFILE_HPP
#define CLANGFILE_HPP

#include "clang/Basic/DiagnosticIDs.h"
#include "clang/Basic/Diagnostic.h"
#include "callback.hpp"
#include "colourscheme.hpp"
#include <QVector>
#include <QTimer>
#include <QThread>
#include <QAbstractItemModel>
#include <QMutex>
#include <QTextCursor>

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Sema/SemaConsumer.h"
#include "clang/Sema/CodeCompleteConsumer.h"

namespace clang {
class TargetInfo;
class Decl;
class Type;
class Stmt;
class ModuleLoader;
class CompilerInvocation;
}

class SemanticWorker;

struct ColourInfo {
	ColourInfo() {}
	ColourInfo(int o, int l, QColor (ColourScheme::*c)() const) :
		offset(o), len(l), fn(c)
	{}
	int offset;
	int len;
	QColor (ColourScheme::*fn)() const;
};
typedef QVector<ColourInfo> ColourVector;

struct DiagInfo {
	DiagInfo() {}
	DiagInfo(int o, int l, clang::DiagnosticsEngine::Level v, QString c) :
		offset(o), len(l), level(v), content(c)
	{}
	int offset;
	int len;
	clang::DiagnosticsEngine::Level level;
	QString content;
};
typedef QVector<DiagInfo> DiagVector;

class ClangFile: public QAbstractListModel, public clang::DiagnosticConsumer, public clang::SemaConsumer, public clang::RecursiveASTVisitor<ClangFile>, public clang::CodeCompleteConsumer {
	Q_OBJECT
public:
	ClangFile();
	~ClangFile();

	void update(const QString& txt);
	void cursorMoved(QTextCursor cur);
	void HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info);
	bool VisitDecl(clang::Decl* decl);
	bool VisitTypeLoc(clang::TypeLoc T);
	bool VisitStmt(clang::Stmt *S);

	int rowCount(const QModelIndex &parent) const;
	QVariant data(const QModelIndex &index, int role) const;

	QMutex mSemanticMutex;
	ColourVector mSemanticColourInfo;
	ColourVector mLexicalColourInfo;
	DiagVector mDiagnosticInfo;
	bool semanticsValid(char sentinel) const { return semanticUpdateSentinel == sentinel+1; }
	bool completionValid(char sentinel) const { return completionUpdateSentinel == sentinel+1; }
	QMutex mCompletionBufferMutex;
	llvm::MemoryBuffer* mCompletionBuffer;

	QStringList mCompletions;
	mutable QMutex mCompletionsMutex;

signals:
	void lexingComplete();
	void parsingComplete(const char sentinel);
	void completionReady(const char sentinel);

protected:
	void lex(llvm::MemoryBuffer* buffer);
	void parse(llvm::MemoryBuffer* buffer, const char sentinel);
	void populateCompletions(const char sentinel, int line, int col);
	bool ignoreDiagnostics;
	clang::DiagnosticConsumer* clone(clang::DiagnosticsEngine &) const;

	char semanticUpdateSentinel;
	char completionUpdateSentinel;
	virtual void HandleTranslationUnit(clang::ASTContext &ctx) {
		TraverseDecl(ctx.getTranslationUnitDecl());
	}
	virtual void ProcessCodeCompleteResults(clang::Sema &S, clang::CodeCompletionContext Context, clang::CodeCompletionResult *Results, unsigned NumResults);
	virtual void ProcessOverloadCandidates(clang::Sema &S, unsigned CurrentArg, OverloadCandidate *Candidates, unsigned NumCandidates);
	virtual clang::CodeCompletionAllocator &getAllocator() { return mCodeCompletionAllocator; }
	clang::CodeCompletionAllocator mCodeCompletionAllocator;

	QVector<llvm::MemoryBuffer*> mMemoryBuffers;
	QTimer mSemanticTimer;

	clang::SourceManager* mTransientSourceManager;
	clang::TargetInfo* targetInfo;
	clang::ModuleLoader* moduleLoader;
	clang::CompilerInvocation* opts;
	QTextCursor mLastCursor;
	bool mHasValidCompletionResults;

public slots:
	void handleTriggerSemanticUpdate();
};


#endif
