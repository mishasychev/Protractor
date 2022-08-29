#include "stdafx.h"

#include "MainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char* argv[])
{
	QApplication a(argc, argv);

	MainWindow w;
	w.showMaximized();
	if (argc > 1)
		w.InitializeFromFile(argv[1]);

	return a.exec();
}