#pragma once
#include <vec3.hpp>

struct surfel_irradiance_information
{
	glm::vec3 pos;
	glm::vec3 color;
	glm::vec3 normal;
	float radius;
	bool split_candidate = false;
	glm::vec3 split_center_1;
	glm::vec3 split_center_2;
};
