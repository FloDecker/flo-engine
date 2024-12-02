#include "Integrator.h"

double Integrator::integrate_euler(const double y_at_x, const double derivative_of_y_at_x, const double step_size)
{
	return y_at_x + step_size * derivative_of_y_at_x;
}

glm::vec3 Integrator::integrate_euler(const glm::vec3 y_at_x, const glm::vec3 derivative_of_y_at_x, const float step_size)
{
	return y_at_x + (derivative_of_y_at_x * step_size);
}
