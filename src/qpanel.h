#ifndef PANEL_HPP
#define PANEL_HPP

#include <QWidget>
class Editor;

// todo flesh this out to
class  Panel : public QWidget {
	
	public:
		Panel(Editor *p = 0);
		virtual ~Panel();
		enum Position
		{
			West,
			North,
			South,
			East
		};
		virtual Position anchor() const { return West; }
		

protected:
		Editor* editor_;
};



#endif // PANEL_HPP
