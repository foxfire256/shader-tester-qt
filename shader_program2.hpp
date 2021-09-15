
#ifndef SHADER_PROGRAM_HPP
#define SHADER_PROGRAM_HPP

#include <string>
#include <list>
#include <memory>
#include <glad/glad.h>

class shader2;
class uniform;

class shader_program2
{
public:
	shader_program2() = default;
	std::string name;
	GLuint id;
	GLint vertex_location;
	GLint normal_location;
	std::unique_ptr<shader2> vertex_shader;
	std::unique_ptr<shader2> geometry_shader;
	std::unique_ptr<shader2> fragment_shader;
	std::list<std::unique_ptr<uniform>> uniforms;
};

#endif
