#ifndef TEXTSTYLE_HPP
#define TEXTSTYLE_HPP

#include <QColor>
#include <QTextCharFormat>

class TextStyle {
public:
	TextStyle() {}
	TextStyle(int s, int l);

	virtual QTextCharFormat toTcf() const = 0;

	int start;
	int len;
};

class HighlightStyle : public TextStyle {
public:
	enum Style { PLAIN = 0, BOLD=1, ITALIC=2 };

	HighlightStyle() {}
	HighlightStyle(QColor c, Style d, int s, int l);

	virtual QTextCharFormat toTcf() const;

private:
	QColor m_colour;
	Style m_style;
};

class DiagStyle : public TextStyle {
public:
	enum Severity { WARNING, ERROR };
	DiagStyle() {}
	DiagStyle(QString message, Severity v, int s, int l);

	virtual QTextCharFormat toTcf() const;
// todo private
public:
	Severity m_severity;
	QString m_message;
};

#include <QVector>
typedef QVector<HighlightStyle> HighlightStyleVector;
typedef QVector<DiagStyle> DiagStyleVector;


#endif // TEXTSTYLE_HPP
