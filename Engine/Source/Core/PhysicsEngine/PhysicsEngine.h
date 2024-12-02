#pragma once
#include <vector>

#include "../Scene/Modifiers/Implementations/PhysicsObjectModifier.h"
#include "IntegrationMethods/euler_integrator.h"


namespace physics_constants
{
	static constexpr float gravity = -9.81f;
	static constexpr auto gravity_vector = glm::vec3(0.0f, gravity, 0.0f);
	
}

class PhysicsEngine
{
public:
	void evaluate_physics_step(double delta_t);
	void register_physics_object(PhysicsObjectModifier* object);

	//integration methods
	euler_integrator* integrator_euler = new euler_integrator();
	
private:
	std::vector<PhysicsObjectModifier*> physics_objects_;
	
};
