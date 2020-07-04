// hacks for NVidia Optimus
// 1 use NVidia, 0 don't unless maybe you set it as default
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
extern "C"
{
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}
#endif

#include <iostream>
#include <string>
#include <chrono>

#include <QApplication>
#include <QFile>
#include <QFont>
#include <QFontDatabase>
#include <QSurfaceFormat>

#include "main_window.hpp"

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	//QCoreApplication::setAttribute(Qt::AA_UseOpenGLES);
	//QCoreApplication::setAttribute(Qt::AA_UseSoftwareOpenGL);

	QApplication a(argc, argv);

	// this does not work on Linux for some reason
	/*
	std::string fnt_name = ":/" + data_root 
		+ "/fonts/grobe_deutschmeister/GrobeDeutschmeister.ttf";
	int id = QFontDatabase::addApplicationFont(fnt_name.c_str());
	QString family = QFontDatabase::applicationFontFamilies(id).at(0);
	// TODO: remove debugging code below
	std::cout << "Font file name:\n";
	std::cout << fnt_name << std::endl;
	std::cout << "Font family: " << family.toStdString() << std::endl;
	QFont fnt(family, 11);
	QApplication::setFont(fnt);
	*/

	// at least specify the font point size for now
	QFont fnt = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
	fnt.setPointSize(11);
	fnt.setBold(true);
	QApplication::setFont(fnt);

	/*
	QFile file(":/style.qss");
	file.open(QFile::ReadOnly);
	QString style_sheet = QLatin1String(file.readAll());
	a.setStyleSheet(style_sheet);
	*/

	// https://doc.qt.io/qt-5/qsurfaceformat.html
	QSurfaceFormat format;
	// set the OpenGL version
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setMajorVersion(4);
	format.setMinorVersion(6);
	// QSurfaceFormat::CompatibilityProfile
	// QSurfaceFormat::CoreProfile
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setSwapBehavior(QSurfaceFormat::DoubleBuffer);
	// 0 = no vertical sync, 1 = vertical sync
	format.setSwapInterval(0);
	QSurfaceFormat::setDefaultFormat(format);

	// if you don't pass nullptr in here the compiler thinks this is a function
	// declaration
	main_window win(nullptr);
	win.setAttribute(Qt::WA_QuitOnClose);
	win.show();

	fflush(stdout);

	int r = a.exec();

	return r;
}
