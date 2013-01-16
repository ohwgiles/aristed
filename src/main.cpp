#include <iostream>
#include <QApplication>
#include "window.hpp"
#include "project.hpp"


int main(int argc, char** argv) {
	QApplication a(argc, argv);

	QList<AeProject*> projects;

	AeWindow gui(projects);
	gui.show();
	return a.exec();
}
