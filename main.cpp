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

#include <boost/version.hpp>
#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/variables_map.hpp>

#include "main_window.hpp"

int main(int argc, char *argv[])
{
	int win_w = 1280;
	int win_h = 768;

	std::string data_root = "";

#if defined(DATA_ROOT)
	data_root = std::string(DATA_ROOT);
#elif !defined(DATA_ROOT) && (defined(_WIN32) || defined(_WIN64))
	std::cout << "Failed to get DATA_ROOT from CMake, using a hard coded value." << std::endl;
	data_root = "C:/dev/shader-tester-qt";
#else
	std::cout << "Failed to get DATA_ROOT from CMake, using a hard coded value." << std::endl;
	data_root = std::string(getenv("HOME")) + "/dev/shader-tester-qt";
#endif

	std::string config_file = "";

	namespace po = boost::program_options;

	po::variables_map vm;
	
	// parse command line
	po::options_description desc("Command line options");

	// WTF how does this work?
	desc.add_options()
		("help,h", "produce help message")
		("width", po::value<int>(), "set window width")
		("height", po::value<int>(), "set window height")
		("config-file", po::value<std::string>(), "specify the config file")
		("data-root", po::value<std::string>(), "set data directory")
		;

	po::store(po::parse_command_line(argc, argv, desc), vm);
	po::notify(vm);

	if(vm.count("help"))
	{
		std::cout << desc << "\n";
		return 1;
	}
	if(vm.count("width"))
	{
		win_w = vm["width"].as<int>();
	}
	if(vm.count("height"))
	{
		win_h = vm["height"].as<int>();
	}
	if(vm.count("config-file"))
	{
		config_file = vm["config-file"].as<std::string>();
	}
	else
	{
		config_file = data_root + "/scene.xml";
	}
	if(vm.count("data-root"))
	{
		data_root = vm["data-root"].as<std::string>();
	}

	std::cout << "DATA_ROOT = " << data_root << std::endl;

	std::cout << "Using Boost "
		// major version
		<< BOOST_VERSION / 100000
		<< "."
		// minor version
		<< BOOST_VERSION / 100 % 1000
		<< "."
		// patch level
		<< BOOST_VERSION % 100
		<< std::endl;

	// WARNING: don't use setFont() when you are using a style sheet
	// this will cause the font to fail to load on Linux
	// https://doc.qt.io/qt-5/qapplication.html#setFont
	//QApplication::setFont(fnt);
	QFile file(QString(data_root.c_str()) + "/style.qss");
	file.open(QFile::ReadOnly);
	QString style_sheet = QLatin1String(file.readAll());

	QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
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

	QApplication a(argc, argv);
	a.setStyleSheet(style_sheet);

	// if you don't pass nullptr in here the compiler thinks this is a function
	// declaration
	main_window win(win_w, win_h, config_file, data_root, nullptr);
	win.setAttribute(Qt::WA_QuitOnClose);
	win.show();

	int r = a.exec();

	return r;
}
