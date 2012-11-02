#ifndef EDITOR_HPP
#define EDITOR_HPP

#include <QPlainTextEdit>
#include "highlighter.hpp"

class CodeModel;
struct ColourScheme;
class LineNumberPanel;
class SearchPanel;
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
	QString filePath() const { return filePath_; }

	bool dirty() const { return mDirty;}

	bool fileExists() const { return fileExists_; }

	bool openFile(QString filename);
	bool saveFile() { return saveFile(filePath_); }
	bool saveFile(QString filename);

	void setCxxModel();

	Highlighter& highlighter() { return hlighter; }

	void setPanelMargins(int l, int t, int r, int b)
	{
	 //m_margins.setCoords(l, t, r, b);
//viewport()->width()
		setViewportMargins(l, t, r, b);

//		if ( flag(LineWrap) )
//	{
//		//qDebug("panel adjust : wrapping to %i", viewport()->width());
//	m_doc->setWidthConstraint(wrapWidth());
//	}
	}

	void showCompletions();
protected:
	const ColourScheme* mColourScheme;
	virtual void keyPressEvent(QKeyEvent *e);
	//virtual const TextStyle* getStyle(int blockNumber, int index);
	virtual bool event(QEvent *e);

private:LineNumberPanel* lnp;
	SearchPanel* searchPanel_;
	bool fileExists_;
	void resizeEvent(QResizeEvent *e);
	QWidget* mLineNumberBar;
	QString filePath_;
	void setDirty(bool b);
	bool mDirty;
	CodeModel* model;
	QCompleter* mCompleter;

	Highlighter hlighter;

	friend class Highlighter;
signals:
	void dirtied(QWidget*,bool);
	void updateCursorPosition(QString);
protected slots:
	void handleDocModified(bool);
	virtual void handleCursorMoved();
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
