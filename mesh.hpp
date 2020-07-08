
#ifndef MESH_HPP
#define MESH_HPP

#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <GL/gl.h>

class mesh
{
public:
	mesh() = default;
	std::string name;
	std::string file_name;
	std::string windows_path;
	std::string linux_path;
	GLuint vertex_vbo;
	GLuint normal_vbo;
};

#endif
