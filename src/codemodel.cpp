#include "codemodel.hpp"

AeCodeModel::AeCodeModel(QObject* parent) :
	QAbstractListModel(parent)
{}

AeCodeModel::IndentType AeCodeModel::getIndentType(QString line) const {
	if(line.startsWith(' '))
		return INDENT_SPACE;
	else
		return INDENT_TAB;
}

int AeCodeModel::getIndentLevel(QString line, AeCodeModel::IndentType type) const {
	QString searchFor = type == INDENT_TAB ? "\t" : QString(spacesPerIndent(),' ');
	int i = 0;
	while(line.startsWith(searchFor)) {
		i++;
		line.remove(0,searchFor.length());
	}
	return i;
}

QString AeCodeModel::indentString(int level, IndentType type) const {
	return type == INDENT_TAB ?
				QString(level, '\t') :
				QString(level * spacesPerIndent(), ' ');
}
