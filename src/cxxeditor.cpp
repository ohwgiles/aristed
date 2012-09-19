#include "cxxeditor.hpp"
#include "colourscheme.hpp"
#include "log.hpp"

#include <QToolTip>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QCompleter>
#include <clang-c/Index.h>

// This class is used so QCompleter can have an AbstractListModel
// to get its results out of. Ideally, CxxEditor would inherit this, but
// then it multiply inherits QObject and things get nasty. So use
// a stupid proxy class.
class CompletionModelProxy : public QAbstractListModel {
public:
	CompletionModelProxy(CxxEditor* e) : e(e) {}
	int rowCount(const QModelIndex &idx) const { return e->rowCount(idx); }
	QVariant data(const QModelIndex &idx, int role) const { return e->data(idx,role); }
private:
	CxxEditor* e;
};

int CxxEditor::rowCount(const QModelIndex &) const {
	return completionResults.size();
}

QVariant CxxEditor::data(const QModelIndex &idx, int role) const {
	if(idx.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
		return completionResults.at(idx.row());
	}
	return QVariant();
}

CxxEditor::CxxEditor(QWidget *parent) :
	Editor(parent),
	ThreadCaller(),
	hlighter(this)
{
	connect(document(), SIGNAL(contentsChange(int,int,int)), this, SLOT(handleTextChanged(int,int,int)));
	setMouseTracking(true);
	mCompletionModel = new CompletionModelProxy(this);
	mCompleter = new QCompleter(mCompletionModel, this);
	mCompleter->setCompletionMode(QCompleter::PopupCompletion);
	mCompleter->setCaseSensitivity(Qt::CaseInsensitive);
	mCompleter->setWidget(this);
	connect(mCompleter, SIGNAL(activated(QString)), this, SLOT(completionChosen(QString)));

	index = clang_createIndex(0,0);
	Editor::connect(&t, SIGNAL(complete(char)), this, SLOT(threadComplete(char)), Qt::QueuedConnection);
}

void CxxEditor::completionChosen(QString repl) {
	mCompleter->popup()->hide();
	QTextCursor tc = textCursor();
	tc.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor, mCompletionPrefix.count());
	tc.removeSelectedText();
	insertPlainText(repl);
}

void CxxEditor::keyPressEvent(QKeyEvent *e) {
	// ctrl-space launches completion widget
	if(e->modifiers() & Qt::CTRL && e->key() == Qt::Key_Space) {
		complete();
		return;
	}
	// If the completion window is available, enter/tab inserts the completion
	if(mCompleter->popup()->isVisible() && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Tab)) {
		completionChosen(mCompleter->currentCompletion());
		return;
	}

	Editor::keyPressEvent(e);
}

bool CxxEditor::event(QEvent *e) {
	if (e->type() == QEvent::ToolTip) {
		QPoint p = viewport()->mapFromGlobal(QCursor::pos());
		QTextCursor tc = cursorForPosition(p);
		int col = tc.columnNumber();
		int row = tc.blockNumber();
		blockDiagnosticStyles.scopedLock();
		for(int i=0, N=blockDiagnosticStyles()[row].size(); i!=N; ++i) {
			const DiagStyle& ds = blockDiagnosticStyles()[row][i];
			if(col > ds.start && col < ds.start + ds.len) {
				QToolTip::showText(QCursor::pos(), ds.m_message, this);
				break;
			} else QToolTip::hideText();
		}
		return true;
	} else
		return Editor::event(e);
}
void CxxEditor::complete() {
	QTextCursor c = mCompletionCursor;
	// TODO sort out the whole CXUnsavedFile fiasco
	CXUnsavedFile thisfile;
	QByteArray b = document()->toPlainText().toUtf8();
	thisfile.Filename = "a.cpp";
	thisfile.Contents = b.constData();
	thisfile.Length = b.size();
	// TODO so far just the name is extracted. We need much more
	CXCodeCompleteResults* results = clang_codeCompleteAt(tu(), "a.cpp",c.blockNumber()+1,c.positionInBlock()+1,&thisfile,1,clang_defaultCodeCompleteOptions());
	completionResults.clear();
	for(unsigned i = 0; i != results->NumResults; ++i) {
		// for function/method completions, index 1 is the name.
		CXString str = clang_getCompletionChunkText(results->Results[i].CompletionString,1);
		completionResults.append(clang_getCString(str));
		clang_disposeString(str);
	}
	clang_disposeCodeCompleteResults(results);

	// Actually display the toolbox
	mCompleter->setCompletionPrefix(mCompletionPrefix);
	// Resetting the model appears to be necessary to get Qt to reproduce the
	// results. Otherwise, when the order of the results changes, the widget
	// will not reiterate them, and will return random results
	mCompleter->setModel(mCompletionModel);
	QRect cr = cursorRect();
	QPoint p = viewport()->mapToParent(cr.topLeft());
	// TODO how wide should it be?
	mCompleter->complete(QRect(p.x(),p.y(), 140, cr.height()));
}

void CxxEditor::handleCursorMoved() {
	// Whenever the cursor is moved, track backwards to find the last
	// non-alphanumeric character. This chunk is the code completion prefix,
	// needed to filter results when the user requests a code completion
	QTextCursor cur = textCursor();
	while(!cur.atBlockStart() && (
			document()->characterAt(cur.position()-1).isLetterOrNumber() ||
			document()->characterAt(cur.position()-1) == QChar('_'))) {
		cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	}
	// Save the cursor, so it can be accessed when the completion is requested
	mCompletionPrefix =  cur.selectedText();
	mCompletionCursor = cur;
	if(mCompleter->popup()->isVisible())
		complete();
}

// Called by the highlighter. It will be repeatedly called with the same
// blockNumber until the function returns zero. Its responsibility is
// to return all the highlighting styles so the highlighter can apply them
// to the document
const TextStyle* CxxEditor::getStyle(int blockNumber, int index) {
	// First do the highlightStyles
	if(blockHighlightStyles()[blockNumber].size() > index)
		return & blockHighlightStyles()[blockNumber][index];
	// Then go onto the diagnostic styles
	else {
		// Since the index is monotonically increasing, we have to first
		// subtract the number of blockHighlightStyles already returned.
		index = index - (*blockHighlightStyles)[blockNumber].size();
		if(blockDiagnosticStyles()[blockNumber].size() > index)
			return & blockDiagnosticStyles()[blockNumber][index];
	}
	// No more styles
	return 0;
}

void CxxEditor::handleTextChanged(int pos, int removed, int added) {
	// When text is first changed, obviously we can't immediately completely
	// reparse the whole translation unit, which is what is required for
	// complete semantic highlighting. Here, we do the best we can: move
	// the colours around so the same things stay highlighted that were
	// before, or if a highlighted word is changed, the highlight is updated
	// to cover the word's new extents. Later the real highlighting algorithm
	// will kick in and correct this.

	// The background thread is kicked off with the value of threadSentinel.
	// Incrementing it causes it to throw away its latest work, since it's
	// just been rendered invalid by another user change.
	threadSentinel++;

	QTextBlock b = document()->findBlock(pos);
	// Theoretically, we could be smarter and deal with multi-line changes,
	// but most of the time where this block of code is useful (fast typing)
	// occurs on a single line. Multi-line is difficult because of the way
	// Qt represents lines as "blocks" in a QTextDocument. This is Good Enough.
	if(b == document()->findBlock(pos + added - removed)) {
		// We have to grab the locks since a background worker thread may be
		// in the process of updating these already. If this is the case, it
		// should discard the result immediately since we've increased the
		// threading sentinel
		blockHighlightStyles.scopedLock();
		blockDiagnosticStyles.scopedLock();

		int relativePos = pos - b.position(); // relative to the current block

		for(int i = 0, N = blockHighlightStyles()[b.blockNumber()].size(); i != N; ++i) {
			TextStyle& ts = (*blockHighlightStyles)[b.blockNumber()][i];
			// if the highlight begins after our cursor position, advance
			// or subtract it by the amount of characters changed
			if(ts.start >= relativePos)
				ts.start += added - removed;
			// if the highlight ends before our cursor position,
			// leave it unchanged
			else if(ts.start + ts.len <= relativePos)
				{}
			// otherwise, we're in the middle of it. extend it. The normal
			// highlighting algorithm will correct this later.
			else
				ts.len += added - removed;
		}
		// We've changed the colours so have to rehighlight this line
		hlighter.rehighlightBlock(b);
	}

	// now store the document contents, to be used by the thread. Access
	// must be protected by a mutex
	lastDocument.scopedLock();
	lastDocument() = document()->toPlainText().toUtf8();
	// Launch the background thread
	triggerThread(threadSentinel);
}


// This function is run in the background thread. It must be careful when
// accessing class members it shares with the main thread
void CxxEditor::doThreadWork(char s) {
	// Fail early if this execution is already invalid
	if(s != threadSentinel) return;

	// make a copy of the document for thread safety
	lastDocument.lock();
	QByteArray documentCopy = *lastDocument;
	lastDocument.unlock();

	// now actually do the processing on the document. This is the part
	// that actually takes up cycles
	CXUnsavedFile thisfile;
	thisfile.Filename = "a.cpp";
	thisfile.Contents = documentCopy.constData();
	thisfile.Length = documentCopy.size();
	const char incpath[] = "-I/usr/lib/clang/" CLANG_VER "/include";
	const char* argv = incpath;
	tu.lock();
	tu() = clang_parseTranslationUnit(index, "a.cpp",&argv, 1, &thisfile, 1, CXTranslationUnit_Incomplete);
	tu.unlock();
	CXCursor Cursor = clang_getTranslationUnitCursor(tu());
	CXSourceRange sr= clang_getCursorExtent(Cursor);
	CXToken* tokens;
	unsigned numtokens;
	clang_tokenize(tu(), sr, &tokens, &numtokens);
	CXCursor* cursors = new CXCursor[numtokens];
	clang_annotateTokens(tu(), tokens, numtokens, cursors);

	// Processing is complete, now convert the clang stuff into appropriate
	// structures to use in the editor. Beginning with the diagnostics
	{ blockDiagnosticStyles.scopedLock();
		blockDiagnosticStyles->clear();
		for(unsigned i=0, n = clang_getNumDiagnostics(tu()); i!=n; ++i) {
			CXDiagnostic diag = clang_getDiagnostic(tu(), i);
			CXDiagnosticSeverity sev = clang_getDiagnosticSeverity(diag);
			CXSourceLocation loc = clang_getDiagnosticLocation(diag);
			CXString str = clang_formatDiagnostic(diag, clang_defaultDiagnosticDisplayOptions());
			unsigned line, col, offset;
			clang_getSpellingLocation(loc,NULL,&line,&col,&offset);
			if(sev == CXDiagnostic_Error) {
				int len = 1;
				while(isalnum(documentCopy.at(offset+len))) len++;
				(*blockDiagnosticStyles)[line-1].append(DiagStyle(clang_getCString(str), DiagStyle::ERROR, col-1, len));
			}
			if(sev == CXDiagnostic_Warning) {

			}
			clang_disposeString(str);
		}
	}

	// And now the syntax highlighting
	{ blockHighlightStyles.scopedLock();
		blockHighlightStyles->clear();

		struct TokenRange {
			int startCol;
			int startLine;
			int endCol;
			int endLine;
		};

		// A multi-line highlighting helper, rather than having to set up
		// another member function (far too many already)
		struct Helper {
			static void appendStyle(HighlightStyleMap& map, QColor c,
							HighlightStyle::Style s, const TokenRange& rng) {
				if(rng.startLine == rng.endLine) // just one line
					map[rng.startLine-1].append(HighlightStyle(c, s, rng.startCol-1, rng.endCol-rng.startCol));
				else { // multiple lines
					int line = rng.startLine -1;
					// first line
					map[line].append(HighlightStyle(c, s, rng.startCol-1, -1));
					// middle lines
					while(++line < rng.endLine-1)
						map[line].append(HighlightStyle(c, s, 0, -1));
					// end line
					map[line].append(HighlightStyle(c, s, 0, rng.endCol-1));
				}
			}
		};

		TokenRange range;
		// loop through all tokens Clang found
		for(unsigned i=0; i<numtokens; ++i) {
			//QTextCharFormat tcf;
			//QColor colour;
			CXCursorKind k =  clang_getCursorKind(cursors[i]);
			CXSourceRange r = clang_getTokenExtent(tu(), tokens[i]);

			clang_getSpellingLocation(clang_getRangeStart(r),NULL,(unsigned*)&range.startLine,(unsigned*)&range.startCol,NULL);
			clang_getSpellingLocation(clang_getRangeEnd(r),NULL,(unsigned*)&range.endLine,(unsigned*)&range.endCol,NULL);
			switch(clang_getTokenKind(tokens[i])) {
			case CXToken_Comment:
				Helper::appendStyle(*blockHighlightStyles, mColourScheme->comment(), HighlightStyle::PLAIN, range);
				break;
			case CXToken_Punctuation:
				break;
			case CXToken_Keyword:
				Helper::appendStyle(*blockHighlightStyles, mColourScheme->keyword(), HighlightStyle::PLAIN, range);
				break;

			case CXToken_Identifier:
				switch(k) {
				case CXCursor_StructDecl:
				case CXCursor_UnionDecl:
				case CXCursor_ClassDecl:
				case CXCursor_EnumDecl:
				case CXCursor_TypedefDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->structure(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_FieldDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->field(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_EnumConstantDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->enumconst(), HighlightStyle::ITALIC, range);
					break;
				case CXCursor_FunctionDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->func(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_VarDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->var(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_ParmDecl:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->param(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_CXXMethod:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->method(), HighlightStyle::PLAIN, range);
					break;
				default:
					break;
				}
				break;


#if 0
				/** \brief A typedef */
				CXCursor_TypedefDecl                   = 20,
						/** \brief A C++ class method. */
						CXCursor_CXXMethod                     = 21,
						/** \brief A C++ namespace. */
						CXCursor_Namespace                     = 22,
						/** \brief A linkage specification, e.g. 'extern "C"'. */
						CXCursor_LinkageSpec                   = 23,
						/** \brief A C++ constructor. */
						CXCursor_Constructor                   = 24,
						/** \brief A C++ destructor. */
						CXCursor_Destructor                    = 25,
						/** \brief A C++ conversion function. */
						CXCursor_ConversionFunction            = 26,
						/** \brief A C++ template type parameter. */
						CXCursor_TemplateTypeParameter         = 27,
						/** \brief A C++ non-type template parameter. */
						CXCursor_NonTypeTemplateParameter      = 28,
						/** \brief A C++ template template parameter. */
						CXCursor_TemplateTemplateParameter     = 29,
						/** \brief A C++ function template. */
						CXCursor_FunctionTemplate              = 30,
						/** \brief A C++ class template. */
						CXCursor_ClassTemplate                 = 31,
						/** \brief A C++ class template partial specialization. */
						CXCursor_ClassTemplatePartialSpecialization = 32,
						/** \brief A C++ namespace alias declaration. */
						CXCursor_NamespaceAlias                = 33,
						/** \brief A C++ using directive. */
						CXCursor_UsingDirective                = 34,
						/** \brief A C++ using declaration. */
						CXCursor_UsingDeclaration              = 35,
						/** \brief A C++ alias declaration */
						CXCursor_TypeAliasDecl                 = 36,
						/** \brief An Objective-C @synthesize definition. */
						CXCursor_ObjCSynthesizeDecl            = 37,
						/** \brief An Objective-C @dynamic definition. */
						CXCursor_ObjCDynamicDecl               = 38,
						/** \brief An access specifier. */
						CXCursor_CXXAccessSpecifier            = 39,
						break;
#endif
			case CXToken_Literal:
				switch(k) {
				case CXCursor_IntegerLiteral:
				case CXCursor_FloatingLiteral:
				case CXCursor_ImaginaryLiteral:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->numericconst(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_StringLiteral:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->string(), HighlightStyle::PLAIN, range);
					break;
				case CXCursor_CharacterLiteral:
					Helper::appendStyle(*blockHighlightStyles, mColourScheme->charliteral(), HighlightStyle::PLAIN, range);
					break;
				default:
					break;
				}

				break;

			default:
				break;
			}
		}
	}

	// free up the clang stuff
	delete[] cursors;
	clang_disposeTokens(tu(), tokens, numtokens);
}

// Called when the thread has finished updating the styles
void CxxEditor::threadComplete(char s) {
	// By this time, the user may have entered input again, invalidating
	// the parsed colours/styles. If so, don't cause a rehighlight, since
	// it is about to be overwritten anyway
	if(s == threadSentinel) {
		blockHighlightStyles.scopedLock();
		blockDiagnosticStyles.scopedLock();
		hlighter.rehighlight();
	}
}
