#include "codedecoration.hpp"


AeCodeDecoration::AeCodeDecoration(QColor colour, Emphasis emphasis, int start, int length) {
	textCharFormat_.setForeground(colour);
	textCharFormat_.setFontItalic(emphasis & ITALIC);
	textCharFormat_.setFontWeight((emphasis & BOLD) ? QFont::Bold : QFont::Normal);
	extents_.start = start;
	extents_.length = length;
}

AeCodeDecoration::AeCodeDecoration(QString message, QColor underline, int start, int length) {
	annotation_ = message;
	textCharFormat_.setUnderlineColor(underline);
	textCharFormat_.setUnderlineStyle(QTextCharFormat::SpellCheckUnderline);
	extents_.start = start;
	extents_.length = length;

}
