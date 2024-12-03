#pragma once
#include <vector>

#include "../Scene/Modifiers/Implementations/mass_spring_point.h"
#include "../Scene/Modifiers/Implementations/physics_object_modifier.h"
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
	void register_physics_object(physics_object_modifier* object);
	bool add_spring(mass_spring_point *point_1, mass_spring_point *point_2, float stiffness);



	//integration methods
	euler_integrator* integrator_euler = new euler_integrator();

	
private:
	std::vector<physics_object_modifier*> physics_objects_; //all physic objects to consider in simulation
	std::vector<spring*> springs_; //springs for mass spring systems
	
};
