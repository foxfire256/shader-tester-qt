#include "main_window.hpp"

#include <iostream>

#include <QTimer>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>

#include "key_handler.hpp"
#include "gl_widget.hpp"

#include "shader_program.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "uniform.hpp"

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

	for(auto &u : (*(glw->sp))->uniforms)
	{
		if((*u)->data_type == "1f")
		{
			float f0;
			try
			{
				f0 = std::stof((*u)->initial_value);
			}
			catch(const std::exception &e)
			{
				std::cerr << "Error: " << e.what() << std::endl;
				std::cerr << "Failed to convert " << (*u)->initial_value << " to float" << std::endl;
				exit(-1);
			}

			glw->u1f.emplace(std::make_pair((*u)->name, f0));

			QLabel *lbl = new QLabel((*u)->name.c_str(), this);
			lbl->setSizePolicy(right_size_policy);
			QLineEdit *le = new QLineEdit((*u)->initial_value.c_str(), this);
			le->setSizePolicy(right_size_policy);
			QHBoxLayout *lt = new QHBoxLayout();
			lt->addWidget(lbl);
			lt->addWidget(le);
			right_layout->addItem(lt);
		}
		else if((*u)->data_type == "vec3")
		{

		}
		else if((*u)->data_type == "vec4")
		{

		}
	}

	main_layout->addWidget(glw);
	main_layout->addItem(right_layout);

	this->setCentralWidget(main_widget);
	this->resize(win_w, win_h);

	kh = new key_handler(this);
	this->installEventFilter(kh);
	connect(kh, &key_handler::quit, this, &main_window::quit);

	connect(glw, &gl_widget::update_fps, this, &main_window::update_fps);
}

//------------------------------------------------------------------------------
main_window::~main_window()
{
	delete glw;
	delete kh;
}

//------------------------------------------------------------------------------
void main_window::quit()
{
	this->close();
}

//------------------------------------------------------------------------------
void main_window::update_fps(int fps)
{
	fps_lbl->setText(QString("FPS: %1").arg(fps));
}



