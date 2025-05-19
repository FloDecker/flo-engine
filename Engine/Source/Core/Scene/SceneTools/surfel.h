#pragma once
#include <vec3.hpp>

struct surfel
{
	glm::vec3 mean;
	glm::vec3 normal;
	glm::vec3 color;
	float radius;
};
