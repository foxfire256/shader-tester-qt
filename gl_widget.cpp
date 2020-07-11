#define _USE_MATH_DEFINES
#include "gl_widget.hpp"

#include <iostream>
#include <fstream>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <boost/range/iterator_range.hpp>

#include "fox/counter.hpp"
#include "fox/gfx/model_loader_obj.hpp"
#include "fox/gfx/eigen_opengl.hpp"
#include "fox/gfx/opengl_error_checker.h"

#include "shader_program.hpp"
#include "shader.hpp"
#include "mesh.hpp"
#include "uniform.hpp"

#if defined(_WIN32) || defined(_WIN64)
	std::string data_root = "C:/dev/model-viewer-data";
#else // Linux or Unix
	std::string data_root = std::string(getenv("HOME")) + "/dev/model-viewer-data";
#endif

gl_widget::gl_widget(const std::string &config_file, QWidget *parent) : QOpenGLWidget(parent)
{
	this->config_file = config_file;
	render_time = 0.0;
	frames = framerate = 0;
	fps_counter = new fox::counter();
	update_counter = new fox::counter();
	obj = nullptr;

	namespace bpt = boost::property_tree;
	bpt::ptree pt;
	try
	{
		bpt::read_xml(config_file, pt, bpt::xml_parser::trim_whitespace);
	}
	catch(const bpt::ptree_error &e)
	{
		std::cerr << "ERROR in: " << __FILE__ << ": at line " << __LINE__
			<< std::endl;
		std::cerr << e.what() << std::endl;
		std::cerr << "Can't open file: " << config_file << std::endl;
		exit(-1);
	}

	for(bpt::ptree::value_type &v : pt.get_child("meshes"))
	{
		mesh *m = nullptr;
		try
		{
			m = new mesh();
			m->name = v.second.get<std::string>("name");
			m->file_name = v.second.get<std::string>("file_name");
			m->windows_path = v.second.get<std::string>("windows_path");
			m->linux_path = v.second.get<std::string>("linux_path");

		}
		catch(const bpt::ptree_error &e)
		{
			std::cerr << "ERROR in: " << __FILE__ << ": at line " << __LINE__
				<< std::endl;
			std::cerr << e.what() << std::endl;
			exit(-1);
		}

		// WARNING: only load one mesh
		this->m = std::make_shared<mesh *>(m);
	}

	for(bpt::ptree::value_type &v : pt.get_child("shader_programs"))
	{
		shader_program *sp = nullptr;
		try
		{
			sp = new shader_program();
			sp->name = v.second.get<std::string>("name");
			sp->vertex_location = v.second.get<GLint>("vertex_location");
			sp->normal_location = v.second.get<GLint>("normal_location");

			for(bpt::ptree::value_type &v2 : v.second.get_child("shaders"))
			{
				shader *s = new shader();
				s->type = v2.second.get<std::string>("type");
				s->file_name = v2.second.get<std::string>("file_name");
				s->windows_path = v2.second.get<std::string>("windows_path");
				s->linux_path = v2.second.get<std::string>("linux_path");
				sp->shaders.push_back(std::make_shared<shader *>(s));
			}

			for(bpt::ptree::value_type &v2 : v.second.get_child("uniforms"))
			{
				uniform *u = new uniform();
				u->name = v2.second.get<std::string>("name");
				u->data_type = v2.second.get<std::string>("data_type");
				u->initial_value = v2.second.get<std::string>("initial_value");
				sp->uniforms.push_back(std::make_shared<uniform *>(u));
			}
		}
		catch(const bpt::ptree_error &e)
		{
			std::cerr << "ERROR in: " << __FILE__ << ": at line " << __LINE__
				<< std::endl;
			std::cerr << e.what() << std::endl;
			exit(-1);
		}

		// WARNING: only have one shader program in the config file
		this->sp = std::make_shared<shader_program *>(sp);
	}

	rot_vel = 16.0f;
}

gl_widget::~gl_widget()
{
	delete fps_counter;
	delete update_counter;
	delete obj;
}

void gl_widget::uniform_changed_1f(const std::string &name, QString s)
{
	std::string s2(s.toStdString());
	
	float f;
	try
	{
		f = std::stof(s2);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Failed to convert " << s2 << " to float" << std::endl;
		return;
	}

	u1f[name] = f;

	print_opengl_error();
	glUseProgram(shader_id);
	int u = glGetUniformLocation(shader_id, name.c_str());
	glUniform1f(u, f);
	print_opengl_error();
}

void gl_widget::uniform_changed_3fv(const std::string &name, int index, QString s)
{
	std::string s2(s.toStdString());
	
	float f;
	try
	{
		f = std::stof(s2);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Failed to convert " << s2 << " to float" << std::endl;
		return;
	}
	
	auto &a = u3fv[name];
	a[index] = f;

	print_opengl_error();
	glUseProgram(shader_id);
	int u = glGetUniformLocation(shader_id, name.c_str());
	glUniform3fv(u, 1, a.data());
	print_opengl_error();
}

void gl_widget::uniform_changed_4fv(const std::string &name, int index, QString s)
{
	std::string s2(s.toStdString());

	float f;
	try
	{
		f = std::stof(s2);
	}
	catch(const std::exception &e)
	{
		std::cerr << "Error: " << e.what() << std::endl;
		std::cerr << "Failed to convert " << s2 << " to float" << std::endl;
		return;
	}

	auto &a = u4fv[name];
	a[index] = f;

	print_opengl_error();
	glUseProgram(shader_id);
	int u = glGetUniformLocation(shader_id, name.c_str());
	glUniform4fv(u, 1, a.data());
	print_opengl_error();
}

void gl_widget::initializeGL()
{
	initializeOpenGLFunctions();

	print_opengl_error();

	printf("GL_VENDOR: %s\n", glGetString(GL_VENDOR));
	printf("GL_RENDERER: %s\n", glGetString(GL_RENDERER));
	printf("GL_VERSION: %s\n", glGetString(GL_VERSION));
	printf("GL_SHADING_LANGUAGE_VERSION: %s\n",
		glGetString(GL_SHADING_LANGUAGE_VERSION));
	//printf("Extensions : %s\n", glGetString(GL_EXTENSIONS));

	fast_vertex_vbo = 0;
	fast_normal_vbo = 0;

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND); // enable alpha channel
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glEnable(GL_POLYGON_SMOOTH);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	// HACK: to get around GLES 2.0 not supporting VAOs and OpenGL 3.2 core
	// requiring one to be bound to use VBOs
#ifndef USING_GLES2
	glGenVertexArrays(1, &default_vao);
	glBindVertexArray(default_vao);
#endif

	trans = { 0.0f, 0.0f, 0.0f };
	rot = { 0.0f, 0.0f, 0.0f };

	eye = Eigen::Vector3f(0.0f, 0.0f, 3.0f);
	target = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	up = Eigen::Vector3f(0.0f, 1.0f, 0.0f);

	fox::gfx::look_at(eye, target, up, V);

	// initialize some defaults
	color = Eigen::Vector4f(1.0f, 1.0f, 1.0f, 1.0f);
	/*
	La = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ls = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ld = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	*/
	rot = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	trans = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	scale = 1.0f;
	//light_pos = Eigen::Vector4f(eye[0], eye[1], eye[2], 1.0f);
	M = Eigen::Matrix4f::Identity();
	fox::gfx::perspective(65.0f, (float)w / (float)h, 0.01f, 40.0f, P);

	MV = V * M;

	normal_matrix(0, 0) = MV(0, 0);
	normal_matrix(0, 1) = MV(0, 1);
	normal_matrix(0, 2) = MV(0, 2);

	normal_matrix(1, 0) = MV(1, 0);
	normal_matrix(1, 1) = MV(1, 1);
	normal_matrix(1, 2) = MV(1, 2);

	normal_matrix(2, 0) = MV(2, 0);
	normal_matrix(2, 1) = MV(2, 1);
	normal_matrix(2, 2) = MV(2, 2);

	MVP = P * MV;

	print_opengl_error();

	//std::string vert_file_name = data_root + "/shaders/phong_v460.vert";
	//std::string frag_file_name = data_root + "/shaders/phong_v460.frag";

	//std::string vert_file_name = data_root + "/shaders/gouraud_v460.vert";
	//std::string frag_file_name = data_root + "/shaders/gouraud_v460.frag";

	std::string vert_file_name = data_root + "/shaders/gouraud_half_vector_v460.vert";
	std::string frag_file_name = data_root + "/shaders/gouraud_half_vector_v460.frag";

	std::ifstream v_f;
	std::ifstream f_f;

	v_f.open(vert_file_name, std::ios::in | std::ios::binary);
	if(!v_f.is_open())
	{
		std::cerr << "Failed to open file: " << vert_file_name << std::endl;
		exit(-1);
	}
	std::string vert_s;
	vert_s.assign(std::istreambuf_iterator<char>(v_f), std::istreambuf_iterator<char>());
	
	f_f.open(frag_file_name, std::ios::in | std::ios::binary);
	if(!f_f.is_open())
	{
		std::cerr << "Failed to open file: " << frag_file_name << std::endl;
		exit(-1);
	}
	std::string frag_s;
	frag_s.assign(std::istreambuf_iterator<char>(f_f), std::istreambuf_iterator<char>());
	
	int length = 0, chars_written = 0;
	char *info_log;

	// vertex shader first
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	if(vertex_shader_id == 0)
	{
		printf("Failed to create GL_VERTEX_SHADER!\n");
		return;
	}

	const char *vert_c = vert_s.c_str();
	glShaderSource(vertex_shader_id, 1, &vert_c, NULL);
	glCompileShader(vertex_shader_id);

	print_shader_info_log(vertex_shader_id);

	fragment_shader_id = glCreateShader(GL_FRAGMENT_SHADER);
	if(fragment_shader_id == 0)
	{
		printf("Failed to create GL_FRAGMENT_SHADER!\n");
		return;
	}

	const char *frag_c = frag_s.c_str();
	glShaderSource(fragment_shader_id, 1, &frag_c, NULL);
	glCompileShader(fragment_shader_id);

	print_shader_info_log(fragment_shader_id);

	shader_id = glCreateProgram();
	if(shader_id == 0)
	{
		printf("Failed at glCreateProgram()!\n");
		return;
	}

	glAttachShader(shader_id, vertex_shader_id);
	glAttachShader(shader_id, fragment_shader_id);

	glLinkProgram(shader_id);

	glGetProgramiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// use 2 for the length because NVidia cards return a line feed always
	if(length > 4)
	{
		info_log = (char *)malloc(sizeof(char) * length);
		if(info_log == NULL)
		{
			printf("ERROR couldn't allocate %d bytes for shader program info log!\n",
				length);
			return;
		}
		else
		{
			printf("Shader program info log:\n");
		}

		glGetProgramInfoLog(shader_id, length, &chars_written, info_log);

		printf("%s\n", info_log);

		free(info_log);
	}

	glUseProgram(shader_id);
	vertex_location = glGetAttribLocation(shader_id, "vertex_position");
	normal_location = glGetAttribLocation(shader_id, "vertex_normal");

	/*
	int u = glGetUniformLocation(shader_id, "light_pos");
	glUniform4fv(u, 1, light_pos.data());
	u = glGetUniformLocation(shader_id, "La");
	glUniform3fv(u, 1, La.data());
	u = glGetUniformLocation(shader_id, "Ls");
	glUniform3fv(u, 1, Ls.data());
	u = glGetUniformLocation(shader_id, "Ld");
	glUniform3fv(u, 1, Ld.data());
	*/
	int u = glGetUniformLocation(shader_id, "color");
	glUniform4fv(u, 1, color.data());

	u = glGetUniformLocation(shader_id, "MVP");
	glUniformMatrix4fv(u, 1, GL_FALSE, MVP.data());
	u = glGetUniformLocation(shader_id, "MV");
	glUniformMatrix4fv(u, 1, GL_FALSE, MV.data());
	u = glGetUniformLocation(shader_id, "normal_matrix");
	glUniformMatrix3fv(u, 1, GL_FALSE, normal_matrix.data());

	print_opengl_error();

	// blade1, bunny, dragon3, icosphere2
	std::string model_file = data_root + "/meshes/dragon3.obj";
	obj = new fox::gfx::model_loader_obj();
	if(obj->load_fast(model_file))
	{
		std::cerr << "Failed to load mesh: " << model_file << std::endl;
		exit(-1);
	}

	printf("Model has %d verticies\n", obj->vertex_count_ogl);
	printf("Vertex data takes up %.3f MB\n", (float)obj->vertex_count_ogl * 12 /
		(1024 * 1024));
	printf("Normal data takes up %.3f MB\n", (float)obj->vertex_count_ogl * 12 /
		(1024 * 1024));

	print_opengl_error();

	/*
	Ka = Eigen::Vector3f(0.3f, 0.3f, 0.3f);
	Ks = Eigen::Vector3f(0.1f, 0.1f, 0.1f);
	Kd = Eigen::Vector3f(0.6f, 0.6f, 0.6f);
	shininess = 5.0f;

	u = glGetUniformLocation(shader_id, "Ka");
	glUniform3fv(u, 1, Ka.data());
	u = glGetUniformLocation(shader_id, "Ks");
	glUniform3fv(u, 1, Ks.data());
	u = glGetUniformLocation(shader_id, "Kd");
	glUniform3fv(u, 1, Kd.data());
	u = glGetUniformLocation(shader_id, "shininess");
	glUniform1f(u, shininess);
	*/
	for(auto i : u1f)
	{
		int u = glGetUniformLocation(shader_id, i.first.c_str());
		glUniform1f(u, i.second);
	}

	for(auto i : u3fv)
	{
		int u = glGetUniformLocation(shader_id, i.first.c_str());
		glUniform3fv(u, 1, i.second.data());
	}

	for(auto i : u4fv)
	{
		int u = glGetUniformLocation(shader_id, i.first.c_str());
		glUniform4fv(u, 1, i.second.data());
	}

	print_opengl_error();

	glGenBuffers(1, &fast_vertex_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fast_vertex_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj->vertex_count_ogl * 3,
		obj->vertices_ogl.data(), GL_STATIC_DRAW);
	glGenBuffers(1, &fast_normal_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, fast_normal_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * obj->normal_count_ogl * 3,
		obj->normals_ogl.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	print_opengl_error();

	fflush(stdout);
	
	this->resizeGL(this->width(), this->height());
}

void gl_widget::resizeGL(int w, int h)
{
	this->w = w;
	this->h = h;

	glViewport(0, 0, w, h);
	fox::gfx::perspective(65.0f, (float)w / (float)h, 0.01f, 40.0f, P);

	MVP = P * MV;

	if(shader_id)
	{
		glUseProgram(shader_id);
		int u = glGetUniformLocation(shader_id, "MVP");
		glUniformMatrix4fv(u, 1, GL_FALSE, MVP.data());
	}
}

void gl_widget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	float dt = update_counter->update();
	rot[1] = rot_vel * dt;
	M = M * Eigen::AngleAxisf(rot[0] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitX());
	M = M * Eigen::AngleAxisf(rot[1] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitY());
	M = M * Eigen::AngleAxisf(rot[2] / 180.0f * (float)M_PI,
		Eigen::Vector3f::UnitZ());

	MV = V * M;

	normal_matrix(0, 0) = MV(0, 0);
	normal_matrix(0, 1) = MV(0, 1);
	normal_matrix(0, 2) = MV(0, 2);

	normal_matrix(1, 0) = MV(1, 0);
	normal_matrix(1, 1) = MV(1, 1);
	normal_matrix(1, 2) = MV(1, 2);

	normal_matrix(2, 0) = MV(2, 0);
	normal_matrix(2, 1) = MV(2, 1);
	normal_matrix(2, 2) = MV(2, 2);

	MVP = P * MV;

	glUseProgram(shader_id);

	int u = glGetUniformLocation(shader_id, "MVP");
	glUniformMatrix4fv(u, 1, GL_FALSE, MVP.data());
	u = glGetUniformLocation(shader_id, "MV");
	glUniformMatrix4fv(u, 1, GL_FALSE, MV.data());
	u = glGetUniformLocation(shader_id, "normal_matrix");
	glUniformMatrix3fv(u, 1, GL_FALSE, normal_matrix.data());

	glBindBuffer(GL_ARRAY_BUFFER, fast_vertex_vbo);
	glEnableVertexAttribArray(vertex_location);
	glVertexAttribPointer(vertex_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ARRAY_BUFFER, fast_normal_vbo);
	glEnableVertexAttribArray(normal_location);
	glVertexAttribPointer(normal_location, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES, 0, obj->vertex_count_ogl);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

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

//------------------------------------------------------------------------------
void gl_widget::print_shader_info_log(GLuint shader_id)
{
	int length = 0, chars_written = 0;
	char *info_log;

	glGetShaderiv(shader_id, GL_INFO_LOG_LENGTH, &length);

	// use 2 for the length because NVidia cards return a line feed always
	if(length > 4)
	{
		info_log = (char *)malloc(sizeof(char) * length);
		if(info_log == NULL)
		{
			printf("ERROR couldn't allocate %d bytes for shader info log!\n",
				length);
			return;
		}

		glGetShaderInfoLog(shader_id, length, &chars_written, info_log);

		printf("Shader info log: %s\n", info_log);

		free(info_log);
	}
}

