#include "clangfile.hpp"

#include "log.hpp"
#include "colourscheme.hpp"
#include "semanticworker.hpp"

#include <QString>
#include <QByteArray>
#include <QMetaType>

#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/Support/Casting.h"

#include "clang/Frontend/DiagnosticOptions.h"
#include "clang/Frontend/TextDiagnosticPrinter.h"

#include "clang/Basic/LangOptions.h"
#include "clang/Basic/FileSystemOptions.h"

#include "clang/Basic/SourceManager.h"
#include "clang/Lex/HeaderSearch.h"
#include "clang/Basic/FileManager.h"

#include "clang/Frontend/HeaderSearchOptions.h"
#include "clang/Frontend/Utils.h"

#include "clang/Basic/TargetOptions.h"
#include "clang/Basic/TargetInfo.h"

#include "clang/Lex/Preprocessor.h"
#include "clang/Frontend/PreprocessorOptions.h"
#include "clang/Frontend/FrontendOptions.h"

#include "clang/Basic/IdentifierTable.h"
#include "clang/Basic/Builtins.h"

#include "clang/AST/ASTContext.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/Sema/Sema.h"
#include "clang/AST/DeclBase.h"
#include "clang/AST/Type.h"
#include "clang/AST/Decl.h"
#include "clang/Sema/Lookup.h"
#include "clang/Sema/Ownership.h"
#include "clang/AST/DeclGroup.h"

#include "clang/Parse/Parser.h"

#include "clang/Parse/ParseAST.h"
#include "clang/Frontend/CompilerInstance.h"

#include "clang/Sema/CodeCompleteConsumer.h"
#include <llvm/Config/config.h>

namespace {
class NullModuleLoader : public clang::ModuleLoader {
	void* loadModule(clang::SourceLocation, clang::IdentifierInfo&, clang::SourceLocation) { return 0; }
};
static SemanticWorker* backgroundWorker = 0;
struct WorkThreadHelper {
	WorkThreadHelper() {
		qRegisterMetaType<SemanticCallback>("SemanticCallback");
		qRegisterMetaType<CompletionCallback>("CompletionCallback");
		backgroundWorker = new SemanticWorker();
	}
};
static WorkThreadHelper _sWorkThreadHelper;
static clang::IgnoringDiagConsumer ignoringConsumer;
}

ClangFile::ClangFile()  :
	QAbstractListModel(),
	clang::DiagnosticConsumer(),
	mCompletionBuffer(0),
	ignoreDiagnostics(false)
{
	opts = new clang::CompilerInvocation();
	opts->setLangDefaults(clang::IK_CXX);
	opts->getLangOpts().Exceptions = 1;
	opts->getLangOpts().CXXExceptions = 1;

	opts->getHeaderSearchOpts().AddPath("/usr/include/c++/" CXX_STDLIB_VER, clang::frontend::CXXSystem, false, false, false);
	opts->getHeaderSearchOpts().AddPath("/usr/include/c++/" CXX_STDLIB_VER "/" LLVM_HOSTTRIPLE, clang::frontend::CXXSystem, false, false, false);
	opts->getHeaderSearchOpts().AddPath("/usr/include/c++/" CXX_STDLIB_VER "/" "backward", clang::frontend::CXXSystem, false, false, false);
	opts->getHeaderSearchOpts().AddPath("/usr/local/include", clang::frontend::Angled, false, false, false);
	opts->getHeaderSearchOpts().AddPath("/usr/include", clang::frontend::Angled, false, false, false);

	ae_debug("/usr/include/c++/" CXX_STDLIB_VER "/" LLVM_HOSTTRIPLE);

	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> dids;
	clang::DiagnosticsEngine de(dids);
	opts->getTargetOpts().Triple = LLVM_HOSTTRIPLE;// llvm::sys::getHostTriple();
	targetInfo = clang::TargetInfo::CreateTargetInfo(de, opts->getTargetOpts());

	moduleLoader = new NullModuleLoader();

	mSemanticTimer.setSingleShot(true);
	connect(&mSemanticTimer, SIGNAL(timeout()), this, SLOT(handleTriggerSemanticUpdate()));

}

ClangFile::~ClangFile() {
	disconnect(this);
	delete moduleLoader;
	delete opts;
}

bool ClangFile::VisitDecl(clang::Decl* decl) {
	if(mTransientSourceManager->getFileID(decl->getLocation()) != mTransientSourceManager->getMainFileID()) return true;
	unsigned start = mTransientSourceManager->getFileOffset(decl->getLocStart()), end = mTransientSourceManager->getFileOffset(decl->getLocEnd());
	end = end == 0 ? start : end;
	ae_info(decl->getDeclKindName() << " " << mTransientSourceManager->getLineNumber(mTransientSourceManager->getMainFileID(), start) << ":" <<
			mTransientSourceManager->getColumnNumber(mTransientSourceManager->getMainFileID(), start) << " to " <<
			mTransientSourceManager->getLineNumber(mTransientSourceManager->getMainFileID(), end) << ":" <<
						mTransientSourceManager->getColumnNumber(mTransientSourceManager->getMainFileID(), end));
	if(clang::CXXMethodDecl* cxm = llvm::dyn_cast<clang::CXXMethodDecl>(decl)) {
		mTransientSourceManager->getFileOffset(cxm->getTypeSourceInfo()->getTypeLoc().getBeginLoc());
		end = mTransientSourceManager->getFileOffset(cxm->getTypeSourceInfo()->getTypeLoc().getEndLoc());
		mSemanticColourInfo.append(ColourInfo(start, end - start, &ColourScheme::comment));

		start = mTransientSourceManager->getFileOffset(cxm->getNameInfo().getLoc());
		end = start + clang::Lexer::MeasureTokenLength(cxm->getNameInfo().getEndLoc(), *mTransientSourceManager, opts->getLangOpts());
		mSemanticColourInfo.append(ColourInfo(start, end-start, &ColourScheme::methodDeclName));
	}
	else if(clang::TypedefDecl* td = llvm::dyn_cast<clang::TypedefDecl>(decl)) {
		ae_debug("got one");
		(void) td;

//		start =  mTransientSourceManager->getFileOffset(td->getLocEnd());
//		mSemanticColourInfo.append(ColourInfo(start,clang::Lexer::MeasureTokenLength(td->getLocEnd(), *mTransientSourceManager, opts->getLangOpts()) , &ColourScheme::methodDeclName));
//		ae_info(mTransientSourceManager->getLineNumber(mTransientSourceManager->getMainFileID(),start) << ":" <<
//				mTransientSourceManager->getColumnNumber(mTransientSourceManager->getMainFileID(), start));
	}
	return true;
}

bool ClangFile::VisitTypeLoc(clang::TypeLoc ) {
//	if(mTransientSourceManager->getFileID(T.getBeginLoc()) != mTransientSourceManager->getMainFileID()) return true;

//	int start = mTransientSourceManager->getFileOffset(T.getLocalSourceRange().getBegin());

//	int end =  mTransientSourceManager->getFileOffset(T.getLocalSourceRange().getEnd());
//	//int end = start + clang::Lexer::MeasureTokenLength(T.getBeginLoc(), *mTransientSourceManager, opts->getLangOpts());
//	ae_info("start at " << mTransientSourceManager->getLineNumber(mTransientSourceManager->getMainFileID(), start) << ":" << mTransientSourceManager->getColumnNumber(mTransientSourceManager->getMainFileID(), start) << ", end at " << mTransientSourceManager->getLineNumber(mTransientSourceManager->getMainFileID(), end) << ":" << mTransientSourceManager->getColumnNumber(mTransientSourceManager->getMainFileID(), end));
//	mSemanticColourInfo.append(ColourInfo(start, end - start, &ColourScheme::comment));

	return true;
}

bool ClangFile::VisitStmt(clang::Stmt *) {
	//S->printPretty();
	return true;
}

clang::DiagnosticConsumer* ClangFile::clone(clang::DiagnosticsEngine &) const {
	ae_fatal("tried to clone ClangFile as a DiagnosticConsumer");
	return 0;
}

void ClangFile::HandleDiagnostic(clang::DiagnosticsEngine::Level level, const clang::Diagnostic &info) {
	DiagnosticConsumer::HandleDiagnostic(level, info);
	if(ignoreDiagnostics) return;
	//if(level < clang::DiagnosticsEngine::Warning) return;
	llvm::SmallVectorImpl<char> str(0);
	info.FormatDiagnostic(str);
	const clang::SourceLocation loc = info.getLocation();
	clang::PresumedLoc ploc = mTransientSourceManager->getPresumedLoc(loc);

	// Decompose the location into a FID/Offset pair.
	std::pair<clang::FileID, unsigned> locInfo = mTransientSourceManager->getDecomposedLoc(loc);
	clang::FileID FID = locInfo.first;
	unsigned FileOffset = locInfo.second;
	if(FID != mTransientSourceManager->getMainFileID()) return;

	unsigned ColNo = mTransientSourceManager->getColumnNumber(FID, FileOffset);
	unsigned CaretendColNo
			= ColNo + clang::Lexer::MeasureTokenLength(loc, *mTransientSourceManager, opts->getLangOpts());

	ae_info("Got diagnostic {" << info.getNumFixItHints() << "} " << ploc.getFilename() << ":" << ploc.getLine() << ":" << ploc.getColumn() << "(" << ColNo << "," << CaretendColNo << ")" << std::string(&str[0], str.size()));
	const clang::DiagnosticIDs& di = *mTransientSourceManager->getDiagnostics().getDiagnosticIDs().getPtr();
	int res = di.getCategoryNumberForDiag(info.getID());
	mDiagnosticInfo.append(DiagInfo(FileOffset, CaretendColNo - ColNo, level, QString::fromLocal8Bit(&str.front(),str.size()) + QString::number(res)));
}

void ClangFile::lex(llvm::MemoryBuffer* buffer) {
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> dids;
	clang::DiagnosticsEngine de(dids);
	clang::FileManager fm(opts->getFileSystemOpts());
	clang::SourceManager sm(de, fm);
	clang::HeaderSearch headerSearch(fm);

	sm.createMainFileIDForMemBuffer(buffer);
	mLexicalColourInfo.clear();

	clang::Preprocessor pp(de, opts->getLangOpts(), targetInfo, sm, headerSearch, *moduleLoader);
	clang::InitializePreprocessor(pp, opts->getPreprocessorOpts(), opts->getHeaderSearchOpts(), opts->getFrontendOpts());
	clang::Lexer lxr(sm.getMainFileID(), buffer, sm, opts->getLangOpts());
	lxr.SetCommentRetentionState(true);
	clang::Token tok;

	lxr.LexFromRawLexer(tok);
	while(tok.isNot(clang::tok::eof)) {
		unsigned tokOffs = sm.getFileOffset(tok.getLocation());
		unsigned tokLen = tok.getLength();
		switch (tok.getKind()) {
		default: break;
		case clang::tok::identifier:
			llvm_unreachable("clang::tok::identifier in raw lexing mode!");
			break;
		case clang::tok::raw_identifier: {
			// Fill in Result.IdentifierInfo and update the token kind,
			// looking up the identifier in the identifier table.
			pp.LookUpIdentifierInfo(tok);

			// If this is a pp-identifier, for a keyword, highlight it as such.
			if (tok.isNot(clang::tok::identifier)) {
				ColourInfo ci( tokOffs, tokLen, &ColourScheme::keyword );
				mLexicalColourInfo.append(ci);
			}
			break;
		}
		case clang::tok::comment: {
			ColourInfo ci( tokOffs, tokLen, &ColourScheme::comment );
			mLexicalColourInfo.append(ci);

		}
			break;
		case clang::tok::utf8_string_literal:
			// Chop off the u part of u8 prefix
			++tokOffs;
			--tokLen;
			// FALL THROUGH to chop the 8
		case clang::tok::wide_string_literal:
		case clang::tok::utf16_string_literal:
		case clang::tok::utf32_string_literal:
			// Chop off the L, u, U or 8 prefix
			++tokOffs;
			--tokLen;
			// FALL THROUGH.
		case clang::tok::string_literal: {
			ColourInfo ci ( tokOffs, tokLen, &ColourScheme::string);
			mLexicalColourInfo.append(ci);}
			break;
		case clang::tok::wide_char_constant:
		case clang::tok::utf16_char_constant:
		case clang::tok::utf32_char_constant:
			// Chop off the L, u, U or 8 prefix
			++tokOffs;
			--tokLen;
		case clang::tok::char_constant:{
			ColourInfo ci ( tokOffs, tokLen, &ColourScheme::charliteral);
			mLexicalColourInfo.append(ci);}
			break;
		case clang::tok::numeric_constant:{
			ColourInfo ci ( tokOffs, tokLen, &ColourScheme::numericconst);
			mLexicalColourInfo.append(ci);}
			break;
		case clang::tok::hash: {
			// If this is a preprocessor directive, all tokens to end of line are too.
			if (!tok.isAtStartOfLine())
				break;
			// Eat all of the tokens until we get to the next one at the start of
			// line.
			unsigned tokEnd = tokOffs+tokLen;
			lxr.LexFromRawLexer(tok);
			while (!tok.isAtStartOfLine() && tok.isNot(clang::tok::eof)) {
				tokEnd = sm.getFileOffset(tok.getLocation())+tok.getLength();
				lxr.LexFromRawLexer(tok);
			}
				ColourInfo ci ( tokOffs, tokEnd-tokOffs, &ColourScheme::preproc);
				mLexicalColourInfo.append(ci);
			continue;
			}
		}
		lxr.LexFromRawLexer(tok);
	}
}

void ClangFile::update(const QString& txt) {
	QByteArray doc8bit = txt.toLocal8Bit();
	ae_info("string: " << std::string(doc8bit.constData(), doc8bit.count()).size() << " 8bitlen: " << doc8bit.count());
	llvm::StringRef sref(doc8bit.constData(), doc8bit.count());
	//llvm::MemoryBuffer* mb = llvm::MemoryBuffer::getMemBufferCopy(sref);
	mMemoryBuffers.append(llvm::MemoryBuffer::getMemBufferCopy(sref));
	mSemanticTimer.start(100);//backgroundWorker->busy() ? 200 : 0);
	lex(llvm::MemoryBuffer::getMemBuffer(sref));
	emit lexingComplete();
}

void ClangFile::handleTriggerSemanticUpdate() {
	Callback<void, llvm::MemoryBuffer*, const char>::Arg2<ClangFile> callback(*this, &ClangFile::parse);
	ae_assert(mMemoryBuffers.count() >= 1);
	for(QVector<llvm::MemoryBuffer*>::iterator it = mMemoryBuffers.begin(); it != mMemoryBuffers.end()-1; ++it)
		delete *it;
	llvm::MemoryBuffer* buf = mMemoryBuffers.back();
	mCompletionBufferMutex.lock(); // save the last good buffer to be used by the code completor
	delete mCompletionBuffer;
	mCompletionBuffer  = llvm::MemoryBuffer::getMemBufferCopy(llvm::StringRef(buf->getBufferStart(),buf->getBufferSize()));
	mCompletionBufferMutex.unlock();
	mMemoryBuffers.clear();
	SemanticCallback scb = { callback, buf, semanticUpdateSentinel++ };
	QMetaObject::invokeMethod(backgroundWorker, "handleSemanticWork", Qt::QueuedConnection, Q_ARG(SemanticCallback, scb));
}

#include <iostream>
void ClangFile::ProcessCodeCompleteResults(clang::Sema &S, clang::CodeCompletionContext , clang::CodeCompletionResult *Results, unsigned NumResults) {
	std::stable_sort(Results, Results + NumResults);
llvm::StringRef sr;
	// Print the results.
	for (unsigned I = 0; I != NumResults; ++I) {
//	  llvm::outs() << "COMPLETION: ";
	  switch (Results[I].Kind) {
	  case clang::CodeCompletionResult::RK_Declaration:
//		llvm::outs() << *Results[I].Declaration;
		sr = Results[I].Declaration->getDeclName().getAsString();
		mCompletions.append(QString::fromLocal8Bit(sr.data(),sr.size()));
//		if (Results[I].Hidden)
//		  llvm::outs() << " (Hidden)";
//		if (clang::CodeCompletionString *CCS
//			  = Results[I].CreateCodeCompletionString(S, mCodeCompletionAllocator)) {
//		  llvm::outs() << " : " << CCS->getAsString();
//		}

//		llvm::outs() << '\n';
		break;

	  case clang::CodeCompletionResult::RK_Keyword:
//		llvm::outs() << Results[I].Keyword << '\n';
		sr = Results[I].Keyword;
		mCompletions.append(QString::fromLocal8Bit(sr.data(),sr.size()));
	  break;

	  case  clang::CodeCompletionResult::RK_Macro: {
		llvm::outs() << Results[I].Macro->getName();
		sr = Results[I].Macro->getName();
		mCompletions.append(QString::fromLocal8Bit(sr.data(),sr.size()));
		if (clang::CodeCompletionString *CCS
			  = Results[I].CreateCodeCompletionString(S, mCodeCompletionAllocator)) {
		  llvm::outs() << " : " << CCS->getAsString();
		}
		llvm::outs() << '\n';
		break;
	  }

	  case clang::CodeCompletionResult::RK_Pattern: {
		llvm::outs() << "Pattern : "
		   << Results[I].Pattern->getAsString() << '\n';
		sr = Results[I].Pattern->getAsString();
		mCompletions.append(QString::fromLocal8Bit(sr.data(),sr.size()));
		break;
	  }
	  }
	}
llvm::outs().flush();
}

void ClangFile::ProcessOverloadCandidates(clang::Sema &S, unsigned CurrentArg, OverloadCandidate *Candidates, unsigned NumCandidates) {
	for (unsigned I = 0; I != NumCandidates; ++I) {
	  if ( clang::CodeCompletionString *CCS
			= Candidates[I].CreateSignatureString(CurrentArg, S,
												  mCodeCompletionAllocator)) {
		llvm::outs() << "OVERLOAD: " << CCS->getAsString() << "\n";
	  }
	}
	llvm::outs().flush();
}

void ClangFile::cursorMoved(QTextCursor cur) {
	ae_debug("cursorMoved");
	if(cur == mLastCursor) return;
	mLastCursor = cur;
	ae_debug("cursorActuallyMoved");

	Callback<void, const char,int,int>::Arg3<ClangFile> callback(*this, &ClangFile::populateCompletions);
	CompletionCallback ccb = { callback, completionUpdateSentinel++, cur.blockNumber(), cur.columnNumber()};
	QMetaObject::invokeMethod(backgroundWorker, "handleCompletionWork", Qt::QueuedConnection, Q_ARG(CompletionCallback, ccb));
}

int ClangFile::rowCount(const QModelIndex &) const {
	mCompletionsMutex.lock();
	int rows = mCompletions.count();
	mCompletionsMutex.unlock();
	return rows;
}
QVariant ClangFile::data(const QModelIndex &index, int role) const {
	if(index.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
		mCompletionsMutex.lock();
		QString result = mCompletions.at(index.row());
		mCompletionsMutex.unlock();
		return result;
	} else return QVariant();
}

/// runs in background thread!
void ClangFile::parse(llvm::MemoryBuffer *buffer, const char sentinel) {
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
	clang::DiagnosticsEngine de(pDiagIDs);
	de.setEnableAllWarnings(true);
	for(clang::DiagnosticIDs::diag_iterator it = clang::DiagnosticIDs::diags_begin(); it != clang::DiagnosticIDs::diags_end(); ++it) {
		if(de.getDiagnosticLevel(it.getDiagID(), clang::SourceLocation()) == clang::DiagnosticsEngine::Fatal)
			de.setDiagnosticMapping(it.getDiagID(), clang::diag::MAP_ERROR, clang::SourceLocation());
	}

	clang::FileManager fileManager(opts->getFileSystemOpts());
	clang::HeaderSearch headerSearch(fileManager);

//	const clang::FileEntry* fe = fileManager.getVirtualFile("", buffer->getBufferSize(), 0);

	clang::SourceManager sourceManager(de, fileManager);
	(void)buffer;
//	sourceManager.overrideFileContents(fe, buffer);
//	sourceManager.createMainFileID(fe);
	mTransientSourceManager = &sourceManager;


	clang::Preprocessor preprocessor(de, opts->getLangOpts(), targetInfo, sourceManager, headerSearch, *moduleLoader);
	clang::InitializePreprocessor(preprocessor, opts->getPreprocessorOpts(), opts->getHeaderSearchOpts(), opts->getFrontendOpts());

	clang::IdentifierTable identifierTable(opts->getLangOpts());
	identifierTable.AddKeywords(opts->getLangOpts()); // todo needed?
	clang::SelectorTable selectorTable;

	clang::Builtin::Context builtinContext;
	builtinContext.InitializeTarget(*targetInfo);
	clang::ASTContext astContext(opts->getLangOpts(), sourceManager, targetInfo, identifierTable, selectorTable, builtinContext, 0);

	//clang::Sema sema(preprocessor, astContext, *this);
	de.setClient(this, false);
//	std::string out;
//	llvm::raw_string_ostream ss(out);
//	clang::DiagnosticConsumer* dc = new clang::TextDiagnosticPrinter(llvm::outs(), opts->getDiagnosticOpts());
//	de.setClient(dc);

	sourceManager.createMainFileIDForMemBuffer(buffer);
	//preprocessor.EnterMainSourceFile();

	mSemanticMutex.lock(); // lock because we need to protect mDiagnosticInfo
	mDiagnosticInfo.clear();
	mSemanticColourInfo.clear();
	BeginSourceFile(opts->getLangOpts(), &preprocessor);
	//dc->BeginSourceFile(opts->getLangOpts(), &preprocessor);
	ae_info("Begin parse");
	//clang::ParseAST(sema);
	clang::ParseAST(preprocessor, this, astContext);
	ae_info("End parse");
	EndSourceFile();
	//dc->EndSourceFile();
	mSemanticMutex.unlock();
	emit parsingComplete(sentinel);
	//llvm::outs() << ss.str();
}

/// runs in background thread!
void ClangFile::populateCompletions(const char sentinel, int line, int col) {
	if(!mCompletionBuffer) return;
	mCompletionBufferMutex.lock();
	llvm::MemoryBuffer* buffer = llvm::MemoryBuffer::getMemBufferCopy(llvm::StringRef(mCompletionBuffer->getBufferStart(),mCompletionBuffer->getBufferSize()));
	mCompletionBufferMutex.unlock();
	ae_debug("populateCompletions");
	llvm::IntrusiveRefCntPtr<clang::DiagnosticIDs> pDiagIDs;
	clang::DiagnosticsEngine de(pDiagIDs);

	clang::FileManager fileManager(opts->getFileSystemOpts());
	clang::HeaderSearch headerSearch(fileManager);
	const clang::FileEntry* fe = fileManager.getVirtualFile("", buffer->getBufferSize(), 0);

	clang::SourceManager sourceManager(de, fileManager);

	sourceManager.overrideFileContents(fe, buffer);
	sourceManager.createMainFileID(fe);

	clang::Preprocessor preprocessor(de, opts->getLangOpts(), targetInfo, sourceManager, headerSearch, *moduleLoader);
	clang::InitializePreprocessor(preprocessor, opts->getPreprocessorOpts(), opts->getHeaderSearchOpts(), opts->getFrontendOpts());

	clang::IdentifierTable identifierTable(opts->getLangOpts());
	identifierTable.AddKeywords(opts->getLangOpts()); // todo needed?
	clang::SelectorTable selectorTable;

	clang::Builtin::Context builtinContext;
	builtinContext.InitializeTarget(*targetInfo);
	clang::ASTContext astContext(opts->getLangOpts(), sourceManager, targetInfo, identifierTable, selectorTable, builtinContext, 0);

	clang::ASTConsumer astc;
	clang::Sema sema(preprocessor, astContext, astc, clang::TU_Complete, this);

	de.setClient(&ignoringConsumer, false);
	ae_debug("ccp: " << line << ":" << col);
	preprocessor.SetCodeCompletionPoint(fe, line+1, col+1);
	//preprocessor.EnterMainSourceFile();
	BeginSourceFile(opts->getLangOpts(), &preprocessor);
	mCompletionsMutex.lock();
	this->beginResetModel();
	mCompletions.clear();

	clang::ParseAST(sema);
	this->endResetModel();
	mCompletionsMutex.unlock();
	EndSourceFile();
	ae_debug("done ccp: ");

	emit completionReady(sentinel);
}












