#pragma once
#include <vec3.hpp>

class math_util
{
public:
	static glm::vec3 barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
	static glm::vec3 get_random_point_in_triangle(glm::vec3 A, glm::vec3 B, glm::vec3 C);
	static glm::vec3 get_tangent(glm::vec3 normal);
	static std::pair<glm::vec3,glm::vec3> get_tangent_and_bi_tangent(glm::vec3 normal);
	static float component_average(glm::vec3 v);
	static glm::vec3 get_weighted_average(const glm::vec3 a, const glm::vec3 b, const float w_a, const float w_b);

};
