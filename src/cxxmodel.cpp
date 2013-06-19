#include <QToolTip>
#include <QAbstractListModel>
#include <QAbstractItemView>
#include <QCompleter>
#include <clang-c/Index.h>
#include <QThread>
#include <QFileInfo>
#include <QDir>
#include "cxxmodel.hpp"
#include "highlighter.hpp"
#include "colourscheme.hpp"
#include "log.hpp"
#include "editor.hpp"


void CxxBackground::launch(char s) {
	(cxxModel_->*memberCallback_)(s);
	emit complete(s);
}

AeCxxModel::AeCxxModel(AeHighlighter &highlighter, const ColourScheme* const&colours, QString filename) : AeCodeModel(),
	mCursor(*new CXCursor),
	highlighter_(highlighter),
	colourScheme_(colours),
	semanticsBuilder_(this, &AeCxxModel::reparseDocument),
	cursorInfoBuilder_(this, &AeCxxModel::findCursorInfo)
{
	clangIndex_ = clang_createIndex(0,0);
	newStyles_ = new StyleMap();
	highlighter_.updateStyles(newStyles_);
	setFileName(filename);

	backgroundWorker_ = new QThread();
	semanticsBuilder_.moveToThread(backgroundWorker_);
	cursorInfoBuilder_.moveToThread(backgroundWorker_);

	connect(&semanticsBuilder_, SIGNAL(complete(char)), this, SLOT(reparseComplete(char)), Qt::QueuedConnection);
	connect(&cursorInfoBuilder_, SIGNAL(complete(char)), this, SLOT(cursorInfoFound(char)), Qt::QueuedConnection);
	backgroundWorker_->start();
}

AeCxxModel::~AeCxxModel() {
	disconnect(this);
	backgroundWorker_->exit();
	backgroundWorker_->wait();
	delete backgroundWorker_;
}

int AeCxxModel::rowCount(const QModelIndex &) const {
	return completionResults_.size();
}

QVariant AeCxxModel::data(const QModelIndex &idx, int role) const {
	if(idx.isValid() && (role == Qt::DisplayRole || role == Qt::EditRole)) {
		return completionResults_.at(idx.row());
	}
	return QVariant();
}

void AeCxxModel::setFileName(QString name) {
	const char* args[7];
	QFileInfo fi(name);

	args[0] = "-x";
	args[1] = "c++";
    args[2] = "-I" ARCH_HEADERS;
	args[3] = "-pedantic";
    args[4] = "-Weverything";
    args[5] = strdup(QString(QString("-I")+fi.dir().path()).toUtf8().constData());
	ae_info(name);
	ae_info(args[2]);
    ae_info(args[5]);

	clangTranslationUnit_.lock();
	fileName_ = name;
	CXUnsavedFile thisfile;
	thisfile.Filename = fileName_.toUtf8().constData();
	thisfile.Contents = 0;//documentCopy.constData();
	thisfile.Length = 0;//documentCopy.size();
	clangTranslationUnit_() = clang_parseTranslationUnit(clangIndex_, name.toLocal8Bit().constData(),
                (const char**)&args, 6, &thisfile, 1,
				CXTranslationUnit_PrecompiledPreamble|CXTranslationUnit_CacheCompletionResults|
				CXTranslationUnit_DetailedPreprocessingRecord|CXTranslationUnit_Incomplete);
	clangTranslationUnit_.unlock();

    free((void*)args[5]);
}


QString AeCxxModel::getTipAt(unsigned int row, unsigned int col) {
	codeAnnotations_.scopedLock();
//	for(int i=0, N=codeAnnotations_()[row].size(); i!=N; ++i) {
//		const Annotation& ds = codeAnnotations_()[row][i];
//        foreach(Annotation::Rng r, ds.ranges)
//            if(col >= r.start && col <= r.start + r.length)
//                return ds.message;
//	}
    ae_debug("gettipat");
    for(int i=0; i<(*newStyles_)[row].size(); ++i) {
        const AeCodeDecoration::Extents& e = (*newStyles_)[row][i].extents();
        if(col >= e.start && col <= e.start + e.length) {
            ae_info("Found annotation: " << (*newStyles_)[row][i].annotation_);
            return (*newStyles_)[row][i].annotation_;
        }
    }

	return "";
}

bool AeCxxModel::keyPressEvent(QPlainTextEdit *editor, QKeyEvent * event) {

	// Simple local function to insert matching chars. Primitively inserts
	// the whole text then backtracks one character
	static struct { bool operator()(QPlainTextEdit *editor, QString txt) const {
		editor->insertPlainText(txt);
		QTextCursor c = editor->textCursor();
		c.movePosition(QTextCursor::PreviousCharacter);
		editor->setTextCursor(c);
		return true;
	}} insertMatched;
	// When placing the left character, auto-fill in the right character
	if(event->key() == Qt::Key_ParenLeft) return insertMatched(editor,"()");
	if(event->key() == Qt::Key_BraceLeft) return insertMatched(editor,"{}");
	if(event->key() == Qt::Key_BracketLeft) return insertMatched(editor,"[]");
	if(event->key() == Qt::Key_QuoteDbl) return insertMatched(editor,"\"\"");
	if(event->key() == Qt::Key_Apostrophe) return insertMatched(editor,"''");

	static struct { QChar operator()(QPlainTextEdit* editor, int p) const {
			int pos = editor->textCursor().position();
			return pos == 0 ? editor->toPlainText().at(pos) : editor->toPlainText().at(pos+p);
	}} chrAt;

	// Debatable: when entering a right character, pass over an existing one
	if(event->key() == Qt::Key_ParenRight && chrAt(editor, 0) == ')') return editor->setTextCursor(QTextCursor(editor->document()->docHandle(), editor->textCursor().position()+1)), true;
	if(event->key() == Qt::Key_BraceLeft) return insertMatched(editor,"{}");
	if(event->key() == Qt::Key_BracketLeft) return insertMatched(editor,"[]");
	if(event->key() == Qt::Key_QuoteDbl) return insertMatched(editor,"\"\"");
	if(event->key() == Qt::Key_Apostrophe) return insertMatched(editor,"''");



	if(event->key() == Qt::Key_Return) {
		QString line = editor->textCursor().block().text();
		IndentType it = getIndentType(line);
		int lastIndent = getIndentLevel(line, it);
		switch(clang_getCursorKind(mCursor)) {
		// For these types of statements, begin the next line +1 indent level
		case CXCursor_IfStmt:
		case CXCursor_ForStmt:
		case CXCursor_WhileStmt:
		case CXCursor_DoStmt:
		case CXCursor_SwitchStmt:
			editor->insertPlainText("\n" + indentString(lastIndent + 1, it));
			break;
		default:
			// Or if the line ends with {, begin next line +1 indent level
			if(chrAt(editor,-1) == '{') {
				editor->insertPlainText("\n" + indentString(lastIndent + 1, it));
				if(chrAt(editor,0) == '}') {
					QString nextLineInsert = "\n" + indentString(lastIndent, it);
					editor->insertPlainText(nextLineInsert);
					editor->setTextCursor(QTextCursor(editor->document()->docHandle(), editor->textCursor().position() - nextLineInsert.length()));
				}
			} else {
				// normal entering: begin next line with the same indent level
				editor->insertPlainText("\n" + indentString(lastIndent, it));
			}
		}
		return true;
	}
	return false;
}

void AeCxxModel::prepareCompletions(QTextDocument* doc) {
	// does not need to lock it since this runs in the same thread as the set
	QTextCursor c = completionCursor_();
	// TODO sort out the whole CXUnsavedFile fiasco
	CXUnsavedFile thisfile;
	QByteArray b = doc->toPlainText().toUtf8();
	thisfile.Filename = fileName_.toUtf8().constData();
	thisfile.Contents = b.constData();
	thisfile.Length = b.size();
	// TODO so far just the name is extracted. We need much more
	CXCodeCompleteResults* results = clang_codeCompleteAt(clangTranslationUnit_(), thisfile.Filename,c.blockNumber()+1,c.positionInBlock()+1,&thisfile,1,clang_defaultCodeCompleteOptions());
	completionResults_.clear();
	for(unsigned i = 0; i != results->NumResults; ++i) {
		// for function/method completions, index 1 is the name.
		CXString str = clang_getCompletionChunkText(results->Results[i].CompletionString,1);
		completionResults_.append(clang_getCString(str));
		clang_disposeString(str);
	}
	clang_disposeCodeCompleteResults(results);

}

void AeCxxModel::cursorPositionChanged(QTextDocument* doc, QTextCursor cur) {
    unsigned col = cur.columnNumber();
    unsigned row = cur.blockNumber();
    ae_info("get tip at " << row << ":" << col);
    for(int i=0; i<(*newStyles_)[row].size(); ++i) {
        const AeCodeDecoration::Extents& e = (*newStyles_)[row][i].extents();
        if(col >= e.start && col <= e.start + e.length) {
            ae_info("Found annotation: " << (*newStyles_)[row][i].annotation_);
            break;
        }
    }


	// Whenever the cursor is moved, track backwards to find the last
	// non-alphanumeric character. This chunk is the code completion prefix,
	// needed to filter results when the user requests a code completion
	while(!cur.atBlockStart() && (
			doc->characterAt(cur.position()-1).isLetterOrNumber() ||
			doc->characterAt(cur.position()-1) == QChar('_'))) {
		cur.movePosition(QTextCursor::PreviousCharacter, QTextCursor::KeepAnchor);
	}
	// Save the cursor, so it can be accessed when the completion is requested
	completionPrefix_ =  cur.selectedText();
	completionCursor_.lock();
	completionCursor_() = cur;
	completionCursor_.unlock();

	QMetaObject::invokeMethod(&cursorInfoBuilder_, "launch", Q_ARG(char, 0));

}

void AeCxxModel::findCursorInfo(char) {

	clangTranslationUnit_.lock();
	CXFile f = clang_getFile(clangTranslationUnit_(), fileName_.toUtf8().constData());
	CXSourceLocation loc = clang_getLocationForOffset(clangTranslationUnit_(),f,completionCursor_().position());
	mCursor = clang_getCursor(clangTranslationUnit_(), loc);

	// todo emit reference information
	clangTranslationUnit_.unlock();

}

void AeCxxModel::handleTextChanged(QTextDocument *document, int position, int removed, int added) {
	// When text is first changed, obviously we can't immediately completely
	// reparse the whole translation unit, which is what is required for
	// complete semantic highlighting. Here, we do the best we can: move
	// the colours around so the same things stay highlighted that were
	// before, or if a highlighted word is changed, the highlight is updated
	// to cover the word's new extents. Later the real highlighting algorithm
	// will kick in and correct this.

	lastDocument_.scopedLock();
	// The background thread is kicked off with the value of threadSentinel.
	// Incrementing it causes it to throw away its latest work, since it's
	// just been rendered invalid by another user change.
	threadSentinel++;

	QTextBlock b = document->findBlock(position);
	// Theoretically, we could be smarter and deal with multi-line changes,
	// but most of the time where this block of code is useful (fast typing)
	// occurs on a single line. Multi-line is difficult because of the way
	// Qt represents lines as "blocks" in a QTextDocument. This is Good Enough.
	if(b == document->findBlock(position + added - removed))
		highlighter_.blockModified(b, position - b.position(), added, removed);

	// now store the document contents, to be used by the thread. Access
	// must be protected by a mutex
	lastDocument_() = document->toPlainText().toUtf8();
	// Launch the background thread
	QMetaObject::invokeMethod(&semanticsBuilder_, "launch", Q_ARG(char, threadSentinel));
}

struct CodePos {
    unsigned int line;
   unsigned int col;
    unsigned int len;
};

static void doOverSourceRange(const CXSourceRange& range, const CXUnsavedFile& document, std::function<void (unsigned line, unsigned column, unsigned length)> fn) {
    unsigned int fl, fc, fo, tl, tc, to;
    CXSourceLocation locFrom = clang_getRangeStart(range);
    clang_getSpellingLocation(locFrom,NULL,&fl,&fc,&fo);
    CXSourceLocation locTo = clang_getRangeEnd(range);
    clang_getSpellingLocation(locTo, NULL, &tl,&tc,&to);
    if(fl == tl)
        fn(fl-1, fc-1, tc-fc);
    else {
        unsigned p = fc;
        while(fl<tl) {
            fo++;
            p++;
            if(document.Contents[fo] == '\n') {
                fn(fl-1,fc-1,p-fc);
                fc = 1;
                fl++;
                p = 0;
            }
        }
        fn(fl-1,0,tc);
    }
}


// this function assumes we have already confirmed the diagnostic indeed belongs to this file
static void markDiagnosticLocations(const CXDiagnostic& diag, const CXUnsavedFile& file, const CXTranslationUnit& tu, QVector<CodePos>& output) {
    CXSourceLocation loc = clang_getDiagnosticLocation(diag);
    unsigned line, col, offset; // these variables will contain the DISPLAY location of the error
    clang_getSpellingLocation(loc,NULL,&line,&col,&offset);
    ae_info("diagnostic incurred locally at " << line << ":" << col);
    // if the diagnostic is at a non-printable location, e.g. no \n at EOF, back it up until it is
    // todo this is a bit fragile.
    unsigned p = offset + 1;
    while(offset > 0 && col > 1 && (offset >= file.Length || file.Contents[offset] == '\n')) { col--; offset--; }
    // calculate the length of the mark
    CXCursorKind ck = clang_getCursorKind(clang_getCursor(tu, loc));
    if(ck >= CXCursor_FirstPreprocessing && ck <= CXCursor_LastPreprocessing) {
        // just mark the whole line, looks better for preproc errors
        while(offset > 0 && file.Contents[offset] != '\n') offset--;
        while(p < file.Length && file.Contents[p] != '\n') p++;
        output.append({line-1,0,p-offset});
        ae_debug("highlighting line at " << line << ", 1:" << p);
    } else {
        // guess the token length by scooping up all alphanumerics
        while(p < file.Length && isalnum(file.Contents[p])) p++;
        ae_debug("highlighting from " << col << " to " << p-offset);
        output.append({line-1,col-1,p-offset});
    }
    // each diagnostic may have supplementary segments to highlight
    // loop through each of them and apply the same process
    for(unsigned i=0, n=clang_getDiagnosticNumRanges(diag); i!=n; ++i) {
        CXSourceRange r = clang_getDiagnosticRange(diag, i);
        doOverSourceRange(r,file,[&](unsigned line,unsigned col,unsigned len){output.append({line,col,len});});
    }


}

// This function is run in the background thread. It must be careful when
// accessing class members it shares with the main thread
void AeCxxModel::reparseDocument(char s) {
	// Fail early if this execution is already invalid
	mutex.lock();
	if(s != threadSentinel) return;

	lastDocument_.lock();
	// make a copy of the document for thread safety
	QByteArray documentCopy = *lastDocument_;
	lastDocument_.unlock();

	// now actually do the processing on the document. This is the part
	// that actually takes up cycles
	CXUnsavedFile thisfile;
	thisfile.Filename = fileName_.toUtf8().constData();
	thisfile.Contents = documentCopy.constData();
	thisfile.Length = documentCopy.size();
	clangTranslationUnit_.lock();
	clang_reparseTranslationUnit(clangTranslationUnit_(), 1, &thisfile, CXTranslationUnit_DetailedPreprocessingRecord|CXTranslationUnit_Incomplete);
	CXCursor Cursor = clang_getTranslationUnitCursor(clangTranslationUnit_());
	CXSourceRange sr= clang_getCursorExtent(Cursor);
	CXToken* tokens;
	unsigned numtokens;
	clang_tokenize(clangTranslationUnit_(), sr, &tokens, &numtokens);
	CXCursor* cursors = new CXCursor[numtokens];
	clang_annotateTokens(clangTranslationUnit_(), tokens, numtokens, cursors);
	clangTranslationUnit_.unlock();

	// Processing is complete, now convert the clang stuff into appropriate
	// structures to use in the editor. Beginning with the diagnostics
	newStyles_->clear();
	codeAnnotations_.lock();
	codeAnnotations_().clear();
	{
		for(unsigned i=0, n = clang_getNumDiagnostics(clangTranslationUnit_()); i!=n; ++i) {
            CXDiagnostic rootDiagnostic = clang_getDiagnostic(clangTranslationUnit_(), i);
            QString diagnosticMessage; // the final diagnostic mesasge, built from many sub-diagnostics

            QVector<CodePos> errorLocations;
            // if the error is caused locally, the root diagnostic will point to this file. if it is in
            // another file, we have to check each of the child diagonstics. Since it is most likely to
            // be in the main file, first tackle that possibility:
            CXSourceLocation loc = clang_getDiagnosticLocation(rootDiagnostic);
            CXFile file;
            clang_getSpellingLocation(loc,&file,NULL,NULL,NULL);
            // todo use CXFileUniqueID when it makes it into distro's clang
            CXString filename = clang_getFileName(file);
            // if the diagnostic is found in the local file
            bool localLocationFound = (strcmp(clang_getCString(filename), fileName_.toLocal8Bit().constData()) == 0);
            clang_disposeString(filename);

            if(localLocationFound)
                markDiagnosticLocations(rootDiagnostic, thisfile, clangTranslationUnit_(), errorLocations);
            else {
                // search through the list of child diagnostics
                CXDiagnosticSet childDiags = clang_getChildDiagnostics(rootDiagnostic);

                for(unsigned i=0, n=clang_getNumDiagnosticsInSet(childDiags); i!=n; ++i) {
                    CXDiagnostic childDiagnostic = clang_getDiagnosticInSet(childDiags, i);
                    // add the child message to the final display diagnostic
                    CXString str = clang_formatDiagnostic(childDiagnostic, clang_defaultDiagnosticDisplayOptions());
                    diagnosticMessage.append(clang_getCString(str));
                    diagnosticMessage.append('\n');
                    clang_disposeString(str);

                    // if we still haven't found the local error location, check if this child diagnostic matches it
                    if(!localLocationFound) {
                        // reusing vars declared earlier
                        CXSourceLocation loc = clang_getDiagnosticLocation(childDiagnostic);
                        clang_getSpellingLocation(loc,&file,NULL,NULL,NULL);
                        // todo use CXFileUniqueID when it makes it into distro's clang
                        CXString filename = clang_getFileName(file);
                        localLocationFound = (strcmp(clang_getCString(filename), fileName_.toLocal8Bit().constData()) == 0);
                        clang_disposeString(filename);
                        if(localLocationFound)
                            markDiagnosticLocations(childDiagnostic, thisfile, clangTranslationUnit_(), errorLocations);


                    }
                }

            }



            { // append the root diagnostic message
                CXString str = clang_formatDiagnostic(rootDiagnostic, clang_defaultDiagnosticDisplayOptions());
                diagnosticMessage.append(clang_getCString(str));
                clang_disposeString(str);
            }

            CXDiagnosticSeverity sev = clang_getDiagnosticSeverity(rootDiagnostic);
            QColor underlineColour;
            if(sev <= CXDiagnostic_Warning)
                underlineColour = colourScheme_->warning();
            else
                underlineColour = colourScheme_->error();
            //ae_info("num ranges: " << an.ranges.count());
            foreach(const CodePos& p, errorLocations) {
                ae_debug("appending diagnostic style at " << p.line << ":" << p.col);
                (*newStyles_)[p.line].append(AeCodeDecoration(diagnosticMessage, underlineColour, p.col, p.len));
            }
            ae_info(diagnosticMessage);
            //an.message = diagnosticMessage;
            //codeAnnotations_()[line-1].append(an);

		}
	}
	codeAnnotations_.unlock();

	// And now the syntax highlighting
	{
        auto StylingHelperFunc = [&](CXSourceRange r, QColor colour, AeCodeDecoration::Emphasis emp){
            doOverSourceRange(r, thisfile, [&](unsigned line, unsigned col, unsigned len){
                (*newStyles_)[line].append(AeCodeDecoration(colour, emp, col, len));
            });
        };

		// loop through all tokens Clang found
		for(unsigned i=0; i<numtokens; ++i) {
            //QTextCharFormat tcf;
            //QColor colour;
            CXCursorKind k =  clang_getCursorKind(cursors[i]);
            CXSourceRange r = clang_getTokenExtent(clangTranslationUnit_(), tokens[i]);
            switch(clang_getTokenKind(tokens[i])) {
			case CXToken_Comment:
                //StylingHelperFunc(r, colourScheme_->comment(), AeCodeDecoration::PLAIN);
                StylingHelperFunc(r, colourScheme_->comment(), AeCodeDecoration::PLAIN);
				break;
			case CXToken_Punctuation:
				break;
			case CXToken_Keyword:
                //StylingHelperFunc(r, colourScheme_->keyword(), AeCodeDecoration::PLAIN);
                StylingHelperFunc(r, colourScheme_->keyword(), AeCodeDecoration::PLAIN);
				break;

			case CXToken_Identifier:
				switch(k) {
				// preprocessor
				case CXCursor_PreprocessingDirective:
				case CXCursor_InclusionDirective:
					// todo backtrack so we also highlight the hash

				case  CXCursor_MacroDefinition:
                    StylingHelperFunc(r, colourScheme_->preproc(), AeCodeDecoration::PLAIN);

					break;
				case CXCursor_MacroExpansion:
                    StylingHelperFunc(r, colourScheme_->preproc(), AeCodeDecoration::BOLD);
					break;
				// Declarations
				case CXCursor_StructDecl:
				case CXCursor_UnionDecl:
				case CXCursor_ClassDecl:
				case CXCursor_EnumDecl:
				case CXCursor_TypedefDecl:
				case CXCursor_ClassTemplate:
                    StylingHelperFunc(r, colourScheme_->structure(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_FieldDecl:
                    StylingHelperFunc(r, colourScheme_->field(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_EnumConstantDecl:
                    StylingHelperFunc(r, colourScheme_->enumconst(), AeCodeDecoration::ITALIC);
					break;
				case CXCursor_FunctionDecl:
                    StylingHelperFunc(r, colourScheme_->func(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_VarDecl:
                    StylingHelperFunc(r, colourScheme_->var(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_ParmDecl:
                    StylingHelperFunc(r, colourScheme_->param(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_LabelStmt:
                    StylingHelperFunc(r, colourScheme_->label(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_CXXMethod:
				case CXCursor_Constructor:
				case CXCursor_Destructor:
				case CXCursor_ConversionFunction: // overriding cast operator

                    StylingHelperFunc(r, colourScheme_->method(),
                             clang_CXXMethod_isVirtual(cursors[i]) ? AeCodeDecoration::ITALIC : AeCodeDecoration::PLAIN);
					break;
				case CXCursor_Namespace:
				case CXCursor_NamespaceAlias:
                    StylingHelperFunc(r, colourScheme_->nspace(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_TemplateTypeParameter:
				case CXCursor_NonTypeTemplateParameter:
				case CXCursor_TemplateTemplateParameter:
				case CXCursor_FunctionTemplate: // TODO where is this used?
				case CXCursor_ClassTemplatePartialSpecialization: // TODO where is this used?
                    StylingHelperFunc(r, colourScheme_->typeParam(), AeCodeDecoration::PLAIN);
					break;
				// References
				case CXCursor_ObjCSuperClassRef:
				case CXCursor_ObjCProtocolRef:
				case CXCursor_ObjCClassRef: // TODO wtf is objective C :)
				case CXCursor_TypeRef:
				case CXCursor_CXXBaseSpecifier:
				case CXCursor_TemplateRef:
                    StylingHelperFunc(r, colourScheme_->structure(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_NamespaceRef:
                    StylingHelperFunc(r, colourScheme_->nspace(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_MemberRef: // TODO where is this used?
				case CXCursor_VariableRef:
                    StylingHelperFunc(r, colourScheme_->field(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_LabelRef:
                    StylingHelperFunc(r, colourScheme_->label(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_OverloadedDeclRef:
				case CXCursor_CallExpr:
                    StylingHelperFunc(r, colourScheme_->func(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_DeclRefExpr:
					switch(clang_getCursorKind(clang_getCursorReferenced(cursors[i]))) {
					case CXCursor_EnumConstantDecl:
                        StylingHelperFunc(r, colourScheme_->enumconst(), AeCodeDecoration::ITALIC);
						break;
					case CXCursor_FunctionDecl:
                        StylingHelperFunc(r, colourScheme_->func(), AeCodeDecoration::PLAIN);
						break;
					case CXCursor_VarDecl:
						// todo it would be nice to make global variable references bold
                        StylingHelperFunc(r, colourScheme_->var(),  AeCodeDecoration::PLAIN);
						break;
					case CXCursor_ParmDecl:
                        StylingHelperFunc(r, colourScheme_->param(), AeCodeDecoration::PLAIN);
						break;
					case CXCursor_CXXMethod:
                        StylingHelperFunc(r, colourScheme_->method(), AeCodeDecoration::PLAIN);
						break;

					default:
						ae_error("Unable to colour type " << clang_getCursorKind(clang_getCursorReferenced(cursors[i])));
						break;
					}
					break;
				case CXCursor_MemberRefExpr:
					switch(clang_getCursorKind(clang_getCursorReferenced(cursors[i]))) {
					case CXCursor_CXXMethod:
					case CXCursor_Constructor:
					case CXCursor_Destructor:
					case CXCursor_ConversionFunction: // overriding cast operator
                        StylingHelperFunc(r, colourScheme_->method(), clang_CXXMethod_isVirtual(clang_getCursorReferenced(cursors[i])) ? AeCodeDecoration::ITALIC : AeCodeDecoration::PLAIN);
						break;
					case CXCursor_FieldDecl:
                        StylingHelperFunc(r, colourScheme_->field(), AeCodeDecoration::PLAIN);
						break;
					default:
						ae_error("Unable to colour type " << clang_getCursorKind(clang_getCursorReferenced(cursors[i])));
						break;
					}
					break;
				case CXCursor_UnexposedExpr:
					ae_error("Unexposed expression");
					break;
				// deliberately not handling
				case CXCursor_CStyleCastExpr:
				case CXCursor_CompoundStmt:
					break;

				default:
					//ae_error("Unhandled syntactic type " << k);
					break;
				}
				break;

			case CXToken_Literal:
				switch(k) {
				case CXCursor_IntegerLiteral:
				case CXCursor_FloatingLiteral:
				case CXCursor_ImaginaryLiteral:
                    StylingHelperFunc(r, colourScheme_->numericconst(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_StringLiteral:
                    StylingHelperFunc(r, colourScheme_->string(), AeCodeDecoration::PLAIN);
					break;
				case CXCursor_CharacterLiteral:
                    StylingHelperFunc(r, colourScheme_->charliteral(), AeCodeDecoration::PLAIN);
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
	clang_disposeTokens(clangTranslationUnit_(), tokens, numtokens);
}

// Called when the thread has finished updating the styles
void AeCxxModel::reparseComplete(char s) {
	// By this time, the user may have entered input again, invalidating
	// the parsed colours/styles. If so, don't cause a rehighlight, since
	// it is about to be overwritten anyway
	if(s == threadSentinel) {
//		blockCodeDecorations.scopedLock();
//		blockDiagnosticStyles.scopedLock();
//		highlighter.rehighlight();
		highlighter_.updateStyles(newStyles_);
	} //else delete newCodeDecorations_;
	mutex.unlock();
}
void AeCxxModel::cursorInfoFound(char) {
	CXString spl = clang_getCursorKindSpelling(clang_getCursorKind(mCursor));
	emit positionInfo(clang_getCString(spl));
	ae_info(clang_getCString(spl));
	clang_disposeString(spl);
}
