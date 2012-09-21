#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QPlainTextEdit>
#include "highlighter.hpp"
class ColourScheme;
class LineNumberBar;
class TextStyle;
class Editor : public QPlainTextEdit {
    Q_OBJECT
public:
	explicit Editor(QWidget *parent = 0);
	virtual void setColourScheme(ColourScheme* scheme);

	QTextCursor wordUnderCursor() const;
	QString displayName() const;
	QString fileName() const { return mFileName; }

    bool dirty() const { return mDirty;}
	bool hasFileName() const { return mHasFileName; }

	bool openFile(QString filename);
	bool saveFile() { return saveFile(mFileName); }
	bool saveFile(QString filename);

protected:
	const ColourScheme* mColourScheme;
	virtual void keyPressEvent(QKeyEvent *e);
	virtual const TextStyle* getStyle(int blockNumber, int index) = 0;

private:
    void resizeEvent(QResizeEvent *e);
	QWidget* mLineNumberBar;
    QString mFileName;
	void setDirty(bool b);
    bool mDirty;
	bool mHasFileName;

	friend class Highlighter;
signals:
	void dirtied(bool);
	void updateCursorPosition(QString);
protected slots:
	 virtual void handleCursorMoved();
	void handleDocModified(bool);

    void updateLineNumberBarWidth(int /* newBlockCount */);
    void updateLineNumberBar(const QRect &rect, int dy);
    void highlightCurrentLine();

private:
	friend class LineNumberBar;
	int lineNumberBarWidth();
	void lineNumberBarPaintEvent(QPaintEvent *event);
};

#endif // EDITOR_HPP
