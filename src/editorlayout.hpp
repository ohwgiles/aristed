
#ifndef EDITORLAYOUT_HPP
#define EDITORLAYOUT_HPP

#include <QLayout>
#include <QList>

class Panel;
class Editor;

class EditorLayout : public QLayout {
	Q_OBJECT
public:
	EditorLayout(Editor *p);
	virtual ~EditorLayout();

	virtual Qt::Orientations expandingDirections() const { return Qt::Horizontal | Qt::Vertical; }
	virtual bool hasHeightForWidth() const { return false; }

	virtual QSize sizeHint() const;
	virtual QSize minimumSize() const;

	virtual QLayoutItem *itemAt(int i) const {
		return (i>=0 && i<layoutItems_.count()) ? layoutItems_.at(i) : 0;
	}
	virtual QLayoutItem *takeAt(int i) {
		return (i>=0 && i<layoutItems_.count()) ? layoutItems_.takeAt(i) : 0;
	}
	virtual int count() const { return layoutItems_.count(); }

public slots:
	virtual void addItem(QLayoutItem *item) { layoutItems_.append(item); }
	virtual void setGeometry(const QRect &rect);

private:
	Editor* editor_;
	QList<QLayoutItem*> layoutItems_;
};

#endif // EDITORLAYOUT_HPP

