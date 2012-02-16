#include <iostream>
#include <QApplication>
#include "mainwindow.hpp"
#include "cxx/clangfile.hpp"

int main(int argc, char** argv) {
	QApplication a(argc, argv);

	MainWindow gui;
	gui.show();
	return a.exec();
}
