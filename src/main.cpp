#include <iostream>
#include <QApplication>
#include "window.hpp"

int main(int argc, char** argv) {
	QApplication a(argc, argv);

	AeWindow gui;
	gui.show();
	return a.exec();
}
