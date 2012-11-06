#ifndef COLOUR_HPP
#define COLOUR_HPP

#include <QColor>

struct ColourScheme {
	virtual QColor background() const = 0;
	virtual QColor panelBackground() const = 0;
	virtual QColor foreground() const = 0;
	virtual QColor currentLineBg() const = 0;
	virtual QColor nspace() const = 0;
	virtual QColor comment()    const = 0;
	virtual QColor string()     const = 0;
	virtual QColor charliteral() const = 0;
	virtual QColor numericconst() const = 0;
	virtual QColor keyword()    const = 0;
	virtual QColor preproc() const = 0;
	virtual QColor method() const = 0;
	virtual QColor structure() const = 0;
	virtual QColor enumconst() const = 0;
	virtual QColor field() const = 0;
	virtual QColor func() const = 0;
	virtual QColor label() const = 0;
	virtual QColor var() const = 0;
	virtual QColor param() const = 0;
	virtual QColor typeParam() const = 0;
	virtual QColor error() const { return Qt::red; }
	virtual QColor warning() const { return Qt::yellow; }
	virtual QColor searchResult() const { return background().lighter(200); }
};

namespace colour {

struct Hinterland : public ColourScheme {
	QColor background()     const { return Qt::black; }
	QColor foreground()     const { return QColor(0xe8,0xe8,0xd3); }
	QColor currentLineBg()  const { return QColor(0x20,0x20,0x20); }
	QColor comment()        const { return QColor(0x88,0x88,0x88); }
	QColor string()         const { return QColor(0x99,0xad,0x6a); }
	QColor charliteral() const { return QColor(0xe1,0xdd,0x85); }
	QColor numericconst() const { return QColor(0xcf,0x6a,0x4c); }
	QColor keyword()        const { return QColor(0x81,0x97,0xbf); }
	QColor preproc() const { return QColor(0x8f,0xbf,0xdc); }
	QColor structure() const { return QColor(0x57,0x90,0x58); }
	QColor enumconst() const { return QColor(0xf3,0xa5,0x5e); }
	QColor method() const { return QColor(0xce,0xe5,0x9f); }
	QColor field() const { return QColor(0x9f,0xce,0xe5); }
	QColor func() const { return QColor(0xb4,0x63,0xcd); }
	QColor var() const { return QColor(0xf1,0xc4,0x3c); }
	QColor param() const { return QColor(0xdd, 0xdc, 0x54); }
	QColor typeParam() const { return Qt::white; }
};

// https://github.com/altercation/solarized
struct Solarized {
	QColor base03() const { return QColor(0x00, 0x2b, 0x36); }
	QColor base02() const { return QColor(0x07, 0x36, 0x42); }
	QColor base01() const { return QColor(0x58, 0x6e, 0x75); }
	QColor base00() const { return QColor(0x65, 0x7b, 0x83); }
	QColor base0() const { return QColor(0x83, 0x94, 0x96); }
	QColor base1() const { return QColor(0x93, 0xa1, 0xa1); }
	QColor base2() const { return QColor(0xee, 0xe8, 0xd5); }
	QColor base3() const { return QColor(0xfd, 0xf6, 0xe3); }
	QColor yellow() const { return QColor(0xb5, 0x89, 0x00); }
	QColor orange() const { return QColor(0xcb, 0x4b, 0x16); }
	QColor red() const { return QColor(0xdc, 0x32, 0x2f); }
	QColor magenta() const { return QColor(0xd3, 0x36, 0x82); }
	QColor violet() const { return QColor(0x6c, 0x71, 0xc4); }
	QColor blue() const { return QColor(0x26, 0x8b, 0xd2); }
	QColor cyan() const { return QColor(0x2a, 0xa1, 0x98); }
	QColor green() const { return QColor(0x85, 0x99, 0x00); }
};

struct SolarizedCommon : public ColourScheme, public Solarized {
	QColor string()         const { return yellow(); }
	QColor charliteral() const { return yellow(); }
	QColor numericconst() const { return yellow(); }
	QColor keyword()        const { return blue(); }
	QColor preproc() const { return orange(); }
	QColor structure() const { return cyan(); }
	QColor enumconst() const { return magenta(); }
	QColor method() const { return violet(); }
	QColor field() const { return emphasised(); }
	QColor func() const { return violet(); }
	virtual QColor emphasised() const = 0;
	QColor var() const { return emphasised(); }
	QColor label() const { return red(); }
	QColor param() const { return emphasised(); }
	QColor nspace() const { return emphasised(); }
	QColor typeParam() const { return magenta(); }
};

struct SolarizedLight : public SolarizedCommon {
	virtual QColor background() const { return base3(); }
	virtual QColor panelBackground() const { return base2(); }
	virtual QColor foreground() const { return base00(); }
	virtual QColor currentLineBg() const { return base2(); }
	virtual QColor comment()    const { return base1(); }
	virtual QColor emphasised() const { return base01(); }
	virtual QColor warning() const { return orange(); }
};

struct SolarizedDark : public SolarizedCommon {
	virtual QColor background() const { return base03(); }
	virtual QColor panelBackground() const { return base02(); }
	virtual QColor foreground() const { return base0(); }
	virtual QColor currentLineBg() const { return base02(); }
	virtual QColor comment()    const { return base01(); }
	virtual QColor emphasised() const { return base1(); }
	virtual QColor warning() const { return yellow(); }

};

}

#endif // COLOUR_HPP
