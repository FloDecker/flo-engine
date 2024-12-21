#pragma once
#include <vec3.hpp>

class math_util
{
public:
	static glm::vec3 barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
};
