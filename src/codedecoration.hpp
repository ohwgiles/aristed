#ifndef CODEDECORATION_HPP
#define CODEDECORATION_HPP
/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
#include <QColor>
#include <QTextCharFormat>

class AeCodeDecoration {
public:
	struct Extents {
        unsigned int start;
        unsigned int length;
	};
	enum Emphasis { PLAIN = 0, BOLD=1, ITALIC=2 };
	AeCodeDecoration() {}
	AeCodeDecoration(QColor colour, Emphasis emphasis, int start, int length);
	AeCodeDecoration(QString message, QColor underline, int start, int length);
	void move(Extents newExtents) { extents_ = newExtents; }
	Extents extents() const { return extents_; }
	const QTextCharFormat& textCharFormat() const { return textCharFormat_; }

    QString annotation_;
private:
	QTextCharFormat textCharFormat_;
	Extents extents_;
};

#include <QVector>
typedef QVector<AeCodeDecoration> StyleVector;
#include <QMap>
typedef QMap<int,StyleVector> StyleMap;

#endif // CODEDECORATION_HPP
