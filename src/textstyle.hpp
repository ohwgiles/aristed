#ifndef TEXTSTYLE_HPP
#define TEXTSTYLE_HPP

#include <QColor>
#include <QTextCharFormat>

class CodeDecoration {
public:
	struct Extents {
		int start;
		int length;
	};
	enum Emphasis { PLAIN = 0, BOLD=1, ITALIC=2 };
	CodeDecoration() {}
	CodeDecoration(QColor colour, Emphasis emphasis, int start, int length);
	CodeDecoration(QString message, QColor underline, int start, int length);
	void move(Extents newExtents) { extents_ = newExtents; }
	Extents extents() const { return extents_; }
	const QTextCharFormat& textCharFormat() const { return textCharFormat_; }

private:
	QTextCharFormat textCharFormat_;
	Extents extents_;
	QString annotation_;
};


#include <QVector>
typedef QVector<CodeDecoration> StyleVector;
#include <QMap>
typedef QMap<int,StyleVector> StyleMap;

#endif // TEXTSTYLE_HPP
