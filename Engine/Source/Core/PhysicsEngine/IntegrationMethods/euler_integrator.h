#pragma once
#include "Integrator.h"

class euler_integrator : public integrator
{
public:
	glm::vec3 integrate(glm::vec3 y_at_x, glm::vec3 derivative_of_y_at_x, float step_size) override;
	glm::vec3 integrate_delta_only(glm::vec3 derivative_of_y_at_x, float step_size) override;
};
