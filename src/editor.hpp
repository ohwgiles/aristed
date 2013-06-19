#ifndef EDITOR_HPP
#define EDITOR_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QPlainTextEdit>

class AeCodeModel;
struct ColourScheme;
class AeLineNumberPanel;
class AeSearchPanel;
class AeDiagnosticPanel;
class TextStyle;
class QCompleter;
class AeHighlighter;
class AeProject;

class AeEditor : public QPlainTextEdit {
	Q_OBJECT

public:
	explicit AeEditor(QWidget *parent = 0);
	~AeEditor();
	virtual void setColourScheme(const ColourScheme *scheme);

	QString displayName() const;
	QString filePath() const { return filePath_; }
	bool dirty() const { return dirty_;}
	bool fileExists() const { return fileExists_; }
	bool openFile(QString filename);
	bool saveFile() { return saveFile(filePath_); }
    bool saveFile(QString filename);

	void setCxxModel();
	AeCodeModel* model() const { return model_; }

	AeProject* project() const { return project_; }
	void setProject(AeProject* project) { project_ = project; }

	// reimplement publicly for linenumberpanel
	QPointF contentOffset() const { return QPlainTextEdit::contentOffset(); }
	void relayout();

signals:
	void dirtied(QWidget*,bool);
	void updateCursorPosition(QString);

private slots:
	virtual void handleCursorMoved();
	void handleTextChanged(int pos, int removed, int added);
	void highlightCurrentLine();
	void completionChosen(QString);
	void searchString(QString);
	void moveToSearchResult(bool);
	void nextSearchResult(bool);
	void prevSearchResult(bool);

private:
	void setDirty(bool b);
	void showCompletions();
	const ColourScheme* colourScheme_;
	virtual void keyPressEvent(QKeyEvent *e);
	virtual bool event(QEvent *e);
	virtual void resizeEvent(QResizeEvent *e);

private:
	AeProject* project_;

	AeLineNumberPanel* lineNumberPanel_;
	AeSearchPanel* searchPanel_;
    AeDiagnosticPanel* diagnosticPanel_;

	AeCodeModel* model_;
	QCompleter* completer_;
	AeHighlighter* highlighter_;

	QString filePath_;
	bool fileExists_;
	bool dirty_;

	enum NavMode { NORMAL, FIND_UNTIL };
	QTextCursor::MoveMode lastMoveMode_;
	NavMode navMode_;

	QRegExp lastSearchTerm_;
	QList<QTextEdit::ExtraSelection> searchResults_;
};

#endif // EDITOR_HPP
