#pragma once
#include <vec3.hpp>

struct struct_collision
{
	bool collision = false;
	glm::vec3 collision_normal = glm::vec3(0, 0, 0);
	glm::vec3 collision_point = glm::vec3(0, 0, 0);
};
