#include "editorlayout.hpp"
#include "qpanel.h"
#include "editor.hpp"

#include <QWidget>
#include <QScrollBar>

EditorLayout::EditorLayout(Editor *p) : QLayout(p), editor_(p) {
	setSpacing(0);
}

EditorLayout::~EditorLayout() {
	QLayoutItem *l;
	while ( (l = takeAt(0)) )
		delete l;
}

QSize EditorLayout::sizeHint() const {
	QSize size;
	for(int i = layoutItems_.count()-1; i >= 0; --i) {
		QLayoutItem *item = layoutItems_.at(i);
		size.rheight() += item->sizeHint().height();
	}
	return size;
}

QSize EditorLayout::minimumSize() const {
	QSize size;
	for(int i = layoutItems_.count()-1; i >= 0; --i) {
		QLayoutItem *item = layoutItems_.at(i);
		size.rheight() += item->minimumSize().height();
	}
	return size;
}

void EditorLayout::setGeometry(const QRect &r) {

	QRect rect(	r.x(), r.y(),
				r.width(),
				r.height()
				);
	
	int i,
		eastWidth = 0,
		westWidth = 0,
		northHeight = 0,
		southHeight = 0,
		centerHeight = 0;
	
	QLayout::setGeometry(rect);
	
	for ( i = 0; i < layoutItems_.size(); ++i )
	{
		QLayoutItem *item = layoutItems_.at(i);
		Panel* p = (Panel*) item->widget();
		Panel::Position position = p->anchor();//wrapper->position;
		
		if ( item->isEmpty()  )
			continue;
		
		if ( position == Panel::North )
		{
			item->setGeometry(QRect(
									rect.x(),
									northHeight,
									rect.width(),
									item->sizeHint().height()
									)
							);
			
			northHeight += item->geometry().height() + spacing();
		} else if (position == Panel::South) {
			item->setGeometry(QRect(item->geometry().x(),
									item->geometry().y(),
									rect.width(),
									item->sizeHint().height()
									)
							);
			
			southHeight += item->geometry().height() + spacing();
			
			item->setGeometry(QRect(rect.x(),
									rect.y() + rect.height() - southHeight + spacing(),
									item->geometry().width(),
									item->geometry().height()
									)
							);
			
		}
	}
	
	centerHeight = rect.height() - northHeight - southHeight;
	
	for ( i = 0; i < layoutItems_.size(); ++i )
	{
		QLayoutItem *item = layoutItems_.at(i);
		Panel *p = (Panel*) item->widget();
		Panel::Position position = p->anchor();
		
		if ( item->isEmpty() )
			continue;
		
		if ( position == Panel::West )
		{
			item->setGeometry(QRect(rect.x() + westWidth,
									northHeight,
									item->sizeHint().width(),
									centerHeight
									)
							);
			
			westWidth += item->geometry().width() + spacing();
		} else if (position == Panel::East) {
			item->setGeometry(QRect(item->geometry().x(),
									item->geometry().y(),
									item->sizeHint().width(),
									centerHeight
									)
							);
			
			eastWidth += item->geometry().width() + spacing();
			
			item->setGeometry(QRect(rect.x() + rect.width() - eastWidth + spacing(),
									northHeight,
									item->geometry().width(),
									item->geometry().height()
									)
							);
			
		}
	}

	editor_->setPanelMargins(westWidth, northHeight, eastWidth, southHeight);
}


