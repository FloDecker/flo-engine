#include "PhysicsEngine.h"

#include <geometric.hpp>

#include "../Scene/Object3D.h"
#include "IntegrationMethods/Integrator.h"

void PhysicsEngine::evaluate_physics_step(double delta_t)
{
	for (auto spring : springs_)
	{
		//calculate spring forces
		auto pos_1 = spring->point_1->get_parent()->getWorldPosition();
		auto pos_2 = spring->point_2->get_parent()->getWorldPosition();
		auto l = glm::distance(pos_1,pos_2);
		auto force_direction = (pos_1-pos_2)/l;
		auto f = -spring->stiffness*(l-spring->initial_length);
		spring->point_1->add_force(force_direction*f);
		spring->point_2->add_force(-force_direction*f);
		
	}
	for (auto physics_object : physics_objects_)
	{
		if (!physics_object->is_fixed)
		{
			physics_object->calculate_forces();
			physics_object->calculate_acceleration();
			physics_object->integrate_velocity(this->integrator_euler, delta_t);
			physics_object->integrate_position(this->integrator_euler, delta_t);
			physics_object->clear_force();
		}
		
	}
}

void PhysicsEngine::register_physics_object(physics_object_modifier* object)
{
	physics_objects_.push_back(object);
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
