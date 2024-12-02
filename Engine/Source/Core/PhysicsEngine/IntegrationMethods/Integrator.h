#pragma once
#include <vec3.hpp>

class Integrator
{
public:
	static double integrate_euler(double y_at_x, double derivative_of_y_at_x , double step_size);
	static glm::vec3 integrate_euler(const glm::vec3 y_at_x, const glm::vec3 derivative_of_y_at_x, const float step_size);
	
};
