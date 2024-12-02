#pragma once
#include <vec3.hpp>

class integrator
{
public:
	virtual ~integrator() = default;
	virtual glm::vec3 integrate(const glm::vec3 y_at_x, const glm::vec3 derivative_of_y_at_x, const float step_size){ return glm::vec3(0);}
	virtual glm::vec3 integrate_delta_only(const glm::vec3 derivative_of_y_at_x, const float step_size){ return glm::vec3(0);}
};
