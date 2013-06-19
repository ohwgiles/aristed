/* Copyright 2013 Oliver Giles
 * This file is part of Aristed. Aristed is licensed under the 
 * GNU GPL version 3. See LICENSE or <http://www.gnu.org/licenses/>
 * for more information */
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
