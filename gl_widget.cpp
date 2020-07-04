
#include "gl_widget.hpp"

#include <iostream>

#include "fox/counter.hpp"
#include "fox/gfx/model_loader_obj.hpp"

#if defined(_WIN32) || defined(_WIN64)
	std::string data_root = "C:/dev/model-viewer-data";
#else // Linux or Unix
	std::string data_root = std::string(getenv("HOME")) + "/dev/model-viewer-data";
#endif

gl_widget::gl_widget(QWidget *parent) : QOpenGLWidget(parent)
{
	render_time = 0.0;
	frames = framerate = 0;
	fps_counter = new fox::counter();
}

gl_widget::~gl_widget()
{
	delete fps_counter;
}

void gl_widget::initializeGL()
{
	initializeOpenGLFunctions();
	
	this->resizeGL(this->width(), this->height());
}

void gl_widget::resizeGL(int w, int h)
{
	this->w = w;
	this->h = h;
}

void gl_widget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);

	frames++;
	
	render_time += fps_counter->update_double();
	if(render_time >= 1.0)
	{
		framerate = frames;
		frames = 0;
		std::cout << "fps: " << framerate << std::endl;
		render_time = 0.0;
		emit update_fps(framerate);
		//print_opengl_error();
	}
}


