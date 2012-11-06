
#ifndef LINENUMBERPANEL_HPP
#define LINENUMBERPANEL_HPP

#include <QWidget>

class AeEditor;
struct ColourScheme;

class AeLineNumberPanel : public QWidget {
public:
	AeLineNumberPanel(AeEditor *e);
	virtual ~AeLineNumberPanel() {}

	void setColourScheme(const ColourScheme* c) { colourScheme_ = c; }

private:
	virtual void paintEvent(QPaintEvent *);

	AeEditor* editor_;
	const ColourScheme* colourScheme_;
};

#endif // LINENUMBERPANEL_HPP

