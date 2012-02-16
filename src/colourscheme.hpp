#ifndef COLOUR_HPP
#define COLOUR_HPP

#include <QColor>

class ColourScheme {
public:
	virtual QColor background() const = 0;
	virtual QColor foreground() const = 0;
	virtual QColor currentLineBg() const = 0;
	virtual QColor comment()    const = 0;
	virtual QColor constant()   const = 0;
	virtual QColor string()     const = 0;
	virtual QColor charliteral() const = 0;
	virtual QColor numericconst() const = 0;
	virtual QColor keyword()    const = 0;
	virtual QColor preproc() const = 0;
	virtual QColor methodDeclName() const = 0;
};

namespace colour {

class Hinterland : public ColourScheme {
public:
	QColor background()     const { return Qt::black; }
	QColor foreground()     const { return QColor(0xe8,0xe8,0xd3); }
	QColor currentLineBg()  const { return QColor(0x20,0x20,0x20); }
	QColor comment()        const { return QColor(0x88,0x88,0x88); }
	QColor constant()       const { return QColor(0xcf,0x6a,0x4c); }
	QColor string()         const { return QColor(0x99,0xad,0x6a); }
	QColor charliteral() const { return QColor(0xe1,0xdd,0x85); }
	QColor numericconst() const { return QColor(0xcf,0x6a,0x4c); }
	QColor keyword()        const { return QColor(0x81,0x97,0xbf); }
	QColor preproc() const { return QColor(0x8f,0xbf,0xdc); }
	virtual QColor methodDeclName() const { return Qt::cyan; }
};

}

#endif // COLOUR_HPP
