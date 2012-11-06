#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QPlainTextEdit>

class AeCodeModel;
struct ColourScheme;
class AeLineNumberPanel;
class AeSearchPanel;
class TextStyle;
class QCompleter;
class AeHighlighter;

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

private:
	void relayout();
	void setDirty(bool b);
	void showCompletions();
	const ColourScheme* colourScheme_;
	virtual void keyPressEvent(QKeyEvent *e);
	virtual bool event(QEvent *e);
	virtual void resizeEvent(QResizeEvent *e);

private:
	AeLineNumberPanel* lineNumberPanel_;
	AeSearchPanel* searchPanel_;

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
