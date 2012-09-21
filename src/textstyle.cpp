#include "textstyle.hpp"

TextStyle::TextStyle(int s, int l) :
	start(s), len(l)
{}

HighlightStyle::HighlightStyle(QColor c, Style d, int s, int l) :
	TextStyle(s, l),
	m_colour(c), m_style(d)
{}

QTextCharFormat HighlightStyle::toTcf() const {
	QTextCharFormat f;
	f.setForeground(m_colour);
	f.setFontItalic(m_style & ITALIC);
	f.setFontWeight((m_style & BOLD) ? QFont::Bold : QFont::Normal);
	return f;
}

DiagStyle::DiagStyle(QString message, QColor underline, int s, int l) :
	TextStyle(s, l),
	m_underline(underline),
	m_message(message)
{}

QTextCharFormat DiagStyle::toTcf() const {
	QTextCharFormat f;
	f.setUnderlineColor(m_underline);
	f.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
	return f;
}
