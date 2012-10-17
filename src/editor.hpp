#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QPlainTextEdit>

#include "highlighter.hpp"
class CodeModel;
struct ColourScheme;
class LineNumberBar;
class TextStyle;
class QCompleter;
class Editor : public QPlainTextEdit {
    Q_OBJECT
public:
	explicit Editor(QWidget *parent = 0);
	~Editor();
	virtual void setColourScheme(ColourScheme* scheme);

	QTextCursor wordUnderCursor() const;
	QString displayName() const;
	QString fileName() const { return mFileName; }

    bool dirty() const { return mDirty;}
	bool hasFileName() const { return mHasFileName; }

	bool openFile(QString filename);
	bool saveFile() { return saveFile(mFileName); }
	bool saveFile(QString filename);

	void setCxxModel();

	Highlighter& highlighter() { return hlighter; }


	void showCompletions();
protected:
	const ColourScheme* mColourScheme;
	virtual void keyPressEvent(QKeyEvent *e);
	virtual const TextStyle* getStyle(int blockNumber, int index);
	virtual bool event(QEvent *e);

private:
    void resizeEvent(QResizeEvent *e);
	QWidget* mLineNumberBar;
    QString mFileName;
	void setDirty(bool b);
    bool mDirty;
	bool mHasFileName;
CodeModel* model;
QCompleter* mCompleter;

Highlighter hlighter;

	friend class Highlighter;
signals:
	void dirtied(bool);
	void updateCursorPosition(QString);
protected slots:
	 virtual void handleCursorMoved();
	void handleDocModified(bool);
void handleTextChanged(int pos, int removed, int added);
    void updateLineNumberBarWidth(int /* newBlockCount */);
    void updateLineNumberBar(const QRect &rect, int dy);
    void highlightCurrentLine();
	 void completionChosen(QString);

private:
	friend class LineNumberBar;
	int lineNumberBarWidth();
	void lineNumberBarPaintEvent(QPaintEvent *event);
};

#endif // EDITOR_HPP
