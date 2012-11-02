#include "searchpanel.hpp"

#include <QLineEdit>
#include "editor.hpp"
#include <QVBoxLayout>
SearchPanel::SearchPanel(Editor *editor) :
	QWidget(editor)
{
	//setFixedHeight(40);

	QVBoxLayout* vbl = new QVBoxLayout();
	vbl->addWidget(new QLineEdit());
	this->setLayout(vbl);

}
