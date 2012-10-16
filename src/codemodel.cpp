#include "codemodel.hpp"

CodeModel::CodeModel(QObject* parent) :
	QAbstractListModel(parent)
{}

CodeModel::~CodeModel() {

}

CodeModel::IndentType CodeModel::getIndentType(QString line) const {
	if(line.startsWith(' '))
		return INDENT_SPACE;
	else
		return INDENT_TAB;
}

int CodeModel::getIndentLevel(QString line, CodeModel::IndentType type) const {
	QString searchFor = type == INDENT_TAB ? "\t" : QString(spacesPerIndent(),' ');
	int i = 0;
	while(line.startsWith(searchFor)) {
		i++;
		line.remove(0,searchFor.length());
	}
	return i;
}

QString CodeModel::indentString(int level, IndentType type) const {
	return type == INDENT_TAB ?
				QString(level, '\t') :
				QString(level * spacesPerIndent(), ' ');
}
