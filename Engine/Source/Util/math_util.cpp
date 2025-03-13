#include "math_util.h"

#include <random>
#include <detail/func_geometric.inl>

glm::vec3 math_util::barycentric(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c)
{
	glm::vec3 v0 = b - a, v1 = c - a, v2 = p - a;
	float d00 = glm::dot(v0, v0);
	float d01 = glm::dot(v0, v1);
	float d11 = glm::dot(v1, v1);
	float d20 = glm::dot(v2, v0);
	float d21 = glm::dot(v2, v1);
	float denom = d00 * d11 - d01 * d01;
	float v = (d11 * d20 - d01 * d21) / denom;
	float w = (d00 * d21 - d01 * d20) / denom;
	float u = 1.0f - v - w;

	return glm::vec3(u, v, w);
}

glm::vec3 math_util::get_random_point_in_triangle(const glm::vec3 A, const glm::vec3 B, const glm::vec3 C) {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_real_distribution<float> dist(0.0f, 1.0f);

	float r1 = dist(gen);
	float r2 = dist(gen);

	if (r1 + r2 > 1) {
		r1 = 1 - r1;
		r2 = 1 - r2;
	}

	auto a = 1 - r1 - r2;
	auto b = r1; 
	auto c = r2;

	return a * A + b * B + c * C;
}