#include "main_window.hpp"

#include <iostream>

#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>

#include "gl_widget.hpp"

//------------------------------------------------------------------------------
main_window::main_window(int win_w, int win_h, const std::string &config_file, 
	QWidget *parent) : QMainWindow(parent)
{
	this->win_w = win_w;
	this->win_h = win_h;
	this->config_file = config_file;
	
	std::cout << "Qt compile time version: " << QT_VERSION_STR << std::endl;
	std::cout << "Qt run time version: " << qVersion() << std::endl;

	glw = new gl_widget(config_file, this);
	glw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	gfx_timer = new QTimer(this);
	// https://doc.qt.io/qt-5/qt.html#TimerType-enum
	gfx_timer->setTimerType(Qt::PreciseTimer);
	connect(gfx_timer, SIGNAL(timeout()), glw, SLOT(update()));
	// a value of 0 here makes this run all the time when the message queue
	// is clear
	gfx_timer->start(0); // time in ms

	QSizePolicy right_size_policy;
	right_size_policy = QSizePolicy(QSizePolicy::Minimum,
		QSizePolicy::Minimum);

	main_widget = new QWidget(this);

	fps_lbl = new QLabel(this);
	fps_lbl->setText("FPS: 000000");

	main_layout = new QHBoxLayout(main_widget);
	right_layout = new QVBoxLayout();

	right_layout->addWidget(fps_lbl);
	right_layout->addStretch();

	main_layout->addWidget(glw);
	main_layout->addItem(right_layout);

	this->setCentralWidget(main_widget);
	this->resize(768, 768);

	connect(glw, &gl_widget::update_fps, this, &main_window::update_fps);
}

//------------------------------------------------------------------------------
main_window::~main_window()
{
	delete glw;
}

//------------------------------------------------------------------------------
void main_window::update_fps(int fps)
{
	fps_lbl->setText(QString("FPS: %1").arg(fps));
}



