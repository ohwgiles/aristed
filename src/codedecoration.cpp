/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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
    textCharFormat_.setUnderlineStyle(QTextCharFormat::WaveUnderline);
	extents_.start = start;
	extents_.length = length;

}
