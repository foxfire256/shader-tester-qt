
#ifndef SHADER2_HPP
#define SHADER2_HPP

#include <string>
#include <glad/glad.h>

class shader2
{
public:
	shader2() = default;
	std::string type;
	std::string file_name;
	GLuint id;
};

#endif
