#include "PhysicsEngine.h"

#include <geometric.hpp>

#include "../Scene/Object3D.h"
#include "IntegrationMethods/Integrator.h"

void PhysicsEngine::evaluate_physics_step(double delta_t)
{
	//calculate spring forces
	for (auto spring : springs_)
	{
		auto pos_1 = spring->point_1->get_parent()->getWorldPosition();
		auto pos_2 = spring->point_2->get_parent()->getWorldPosition();
		auto l = glm::distance(pos_1,pos_2);
		auto force_direction = (pos_1-pos_2)/l;
		auto f = -spring->stiffness*(l-spring->initial_length);
		spring->point_1->add_force(force_direction*f);
		spring->point_2->add_force(-force_direction*f);
		
	}

	//evaluate rigid body
	

	//evaluate mass spring system
	for (auto mass_spring : mass_spring_objects_)
	{
		if (!mass_spring->is_fixed)
		{
			mass_spring->calculate_forces();
			mass_spring->calculate_acceleration();
			mass_spring->integrate_velocity(this->integrator_euler, delta_t);
			mass_spring->integrate_position(this->integrator_euler, delta_t);
			mass_spring->clear_force();
		}
	}
	
}

void PhysicsEngine::register_mass_spring_object(physics_object_modifier* object)
{
	mass_spring_objects_.push_back(object);
}

bool PhysicsEngine::add_spring(mass_spring_point* point_1, mass_spring_point* point_2, float stiffness)
{
	for (auto spring : springs_)
	{
		if ((spring->point_1 == point_1 && spring->point_2 == point_2) ||
			(spring->point_1 == point_2 && spring->point_2 == point_1))
		{
			//connection already exists
			return false;
		}
	}
	const auto distance = glm::distance(point_1->get_parent()->getWorldPosition(),
	                                    point_2->get_parent()->getWorldPosition());
	const auto new_spring = new spring(point_1, point_2, stiffness, distance);
	springs_.push_back(new_spring);
	return true;
}
