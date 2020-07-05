
#ifndef UNIFORM_HPP
#define UNIFORM_HPP

#include <string>

class uniform
{
public:
	std::string name;
	// currently only 1f, vec3, vec4
	std::string data_type;
	// comma separated values for vectors
	std::string initial_value;
};

#endif
