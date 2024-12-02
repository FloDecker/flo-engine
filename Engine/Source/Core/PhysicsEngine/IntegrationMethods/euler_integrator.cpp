#include "euler_integrator.h"

glm::vec3 euler_integrator::integrate(const glm::vec3 y_at_x, const glm::vec3 derivative_of_y_at_x,
	const float step_size)
{
	return y_at_x + derivative_of_y_at_x * step_size;
}

glm::vec3 euler_integrator::integrate_delta_only(const glm::vec3 derivative_of_y_at_x, const float step_size)
{
	return derivative_of_y_at_x * step_size;
}
