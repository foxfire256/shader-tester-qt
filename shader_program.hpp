
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
	GLuint program_id;
	GLint vertex_location;
	GLint normal_location;
	std::list<std::shared_ptr<shader *>> shaders;
	std::list<std::shared_ptr<uniform *>> uniforms;
};

#endif
