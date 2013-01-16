#ifndef CODEDECORATION_HPP
#define CODEDECORATION_HPP

#include <QColor>
#include <QTextCharFormat>

class AeCodeDecoration {
public:
	struct Extents {
		int start;
		int length;
	};
	enum Emphasis { PLAIN = 0, BOLD=1, ITALIC=2 };
	AeCodeDecoration() {}
	AeCodeDecoration(QColor colour, Emphasis emphasis, int start, int length);
	AeCodeDecoration(QString message, QColor underline, int start, int length);
	void move(Extents newExtents) { extents_ = newExtents; }
	Extents extents() const { return extents_; }
	const QTextCharFormat& textCharFormat() const { return textCharFormat_; }

private:
	QTextCharFormat textCharFormat_;
	Extents extents_;
	QString annotation_;
};

#include <QVector>
typedef QVector<AeCodeDecoration> StyleVector;
#include <QMap>
typedef QMap<int,StyleVector> StyleMap;

#endif // CODEDECORATION_HPP