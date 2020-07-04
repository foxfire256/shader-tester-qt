#define _USE_MATH_DEFINES
#include "gl_widget.hpp"

#include <iostream>

#include "fox/counter.hpp"
#include "fox/gfx/model_loader_obj.hpp"
#include "fox/gfx/eigen_opengl.hpp"
#include "fox/gfx/opengl_error_checker.h"

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
	update_counter = new fox::counter();
	obj = nullptr;

	rot_vel = 16.0f;
}

gl_widget::~gl_widget()
{
	delete fps_counter;
	delete update_counter;
	delete obj;
}

void gl_widget::initializeGL()
{
	initializeOpenGLFunctions();

	print_opengl_error();

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
	La = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ls = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	Ld = Eigen::Vector3f(1.0f, 1.0f, 1.0f);
	rot = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	trans = Eigen::Vector3f(0.0f, 0.0f, 0.0f);
	scale = 1.0f;
	light_pos = Eigen::Vector4f(eye[0], eye[1], eye[2], 1.0f);
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

	char vert_file[256], frag_file[256];
	//snprintf(vert_file, 256, "%s/shaders/phong_v460.vert", data_root.c_str());
	//snprintf(frag_file, 256, "%s/shaders/phong_v460.frag", data_root.c_str());

	//snprintf(vert_file, 256, "%s/shaders/gouraud_v460.vert", data_root.c_str());
	//snprintf(frag_file, 256, "%s/shaders/gouraud_v460.frag", data_root.c_str());

	snprintf(vert_file, 256, "%s/shaders/gouraud_half_vector_v460.vert", data_root.c_str());
	snprintf(frag_file, 256, "%s/shaders/gouraud_half_vector_v460.frag", data_root.c_str());

	FILE *v, *f;
	char *vert, *frag;
	int fsize, vsize, result;

	v = fopen(vert_file, "rt");
	if(v == NULL)
	{
		printf("ERROR couldn't open vertex shader file %s\n", vert_file);
		return;
	}

	f = fopen(frag_file, "rt");
	if(f == NULL)
	{
		printf("ERROR couldn't open vertex shader file %s\n", frag_file);
		return;
	}

	// find the file size
	fseek(v, 0, SEEK_END);
	vsize = ftell(v);
	rewind(v);

	fseek(f, 0, SEEK_END);
	fsize = ftell(f);
	rewind(f);

	vert = (char *)malloc(sizeof(char) * vsize);
	frag = (char *)malloc(sizeof(char) * fsize);

	// read the file
	result = (int)fread(vert, sizeof(char), vsize, v);
	vert[vsize - 1] = '\0';
	if(result != vsize)
	{
		printf("ERROR: loading shader: %s\n", vert_file);
		printf("Expected %d bytes but only read %d\n", vsize, result);

		fclose(f);
		fclose(v);
		free(vert);
		free(frag);
		return;
	}

	result = (int)fread(frag, sizeof(char), fsize, f);
	frag[fsize - 1] = '\0';
	if(result != fsize)
	{
		printf("ERROR: loading shader: %s\n", frag_file);
		printf("Expected %d bytes but only read %d\n", fsize, result);

		fclose(f);
		fclose(v);
		free(vert);
		free(frag);
		return;
	}

	fclose(f);
	fclose(v);

	int length = 0, chars_written = 0;
	char *info_log;

	// vertex shader first
	vertex_shader_id = glCreateShader(GL_VERTEX_SHADER);
	if(vertex_shader_id == 0)
	{
		printf("Failed to create GL_VERTEX_SHADER!\n");
		return;
	}

	std::string vert_s = vert;
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

	std::string frag_s = frag;
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

	free(vert);
	free(frag);

	glUseProgram(shader_id);
	vertex_location = glGetAttribLocation(shader_id, "vertex_position");
	normal_location = glGetAttribLocation(shader_id, "vertex_normal");

	int u = glGetUniformLocation(shader_id, "light_pos");
	glUniform4fv(u, 1, light_pos.data());
	u = glGetUniformLocation(shader_id, "La");
	glUniform3fv(u, 1, La.data());
	u = glGetUniformLocation(shader_id, "Ls");
	glUniform3fv(u, 1, Ls.data());
	u = glGetUniformLocation(shader_id, "Ld");
	glUniform3fv(u, 1, Ld.data());

	u = glGetUniformLocation(shader_id, "color");
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

