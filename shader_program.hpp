
#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <string>
#include <list>
#include <memory>
#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif
#include <GL/gl.h>

class shader;
class uniform;

class shader_program
{
public:
	shader_program() = default;
	std::string name;
	GLuint id;
	GLint vertex_location;
	GLint normal_location;
	std::unique_ptr<shader> vertex_shader;
	std::unique_ptr<shader> geometry_shader;
	std::unique_ptr<shader> fragment_shader;
	std::list<std::unique_ptr<uniform>> uniforms;
};

#endif
