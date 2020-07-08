
#ifndef SHADER_HPP
#define SHADER_HPP

#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <GL/gl.h>

class shader
{
public:
	shader() = default;
	std::string type;
	std::string file_name;
	std::string windows_path;
	std::string linux_path;
	GLuint shader_id;
};

#endif
