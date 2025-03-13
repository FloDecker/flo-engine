#pragma once
#include <vector>

#include "../Scene/Modifiers/Implementations/mass_spring_point.h"
#include "../Scene/Modifiers/Implementations/physics_object_modifier.h"
#include "../Scene/Modifiers/Implementations/rigid_body.h"
#include "../Scene/Modifiers/Implementations/Colliders/collider_modifier.h"
#include "IntegrationMethods/euler_integrator.h"


class Scene;

namespace physics_constants
{
	static constexpr float gravity = -9.81f;
	static constexpr auto gravity_vector = glm::vec3(0.0f, gravity, 0.0f);
	
}

class PhysicsEngine
{
public:
	explicit PhysicsEngine(Scene* scene)
		: scene_(scene)
	{
	}

	void evaluate_physics_step(double delta_t);

	// mass spring
	void register_mass_spring_object(physics_object_modifier* object);
	bool add_spring(mass_spring_point *point_1, mass_spring_point *point_2, float stiffness);

	//rigid body
	void register_rigid_body(rigid_body* rigid_body);
	
	//integration methods
	euler_integrator* integrator_euler = new euler_integrator();

	
private:
	std::vector<physics_object_modifier*> mass_spring_objects_; //all physic objects to consider in simulation
	std::vector<spring*> springs_; //springs for mass spring systems

	std::vector<rigid_body*> rigid_bodies_;
	
	Scene *scene_;
};
