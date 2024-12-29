#pragma once
#include <vec3.hpp>

struct StructColorRange
{
	int sample_amount;
	std::vector<glm::vec3> colors;
	std::vector<float> sample_points;
};
