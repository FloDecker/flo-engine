#include "PhysicsEngine.h"

#include <geometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../Scene/Object3D.h"
#include "IntegrationMethods/Integrator.h"

void PhysicsEngine::evaluate_physics_step(double delta_t)
{

	//collision detection
	size_t numColliders = colliders_.size();
    
	for (size_t i = 0; i < numColliders; ++i) {
		for (size_t j = i + 1; j < numColliders; ++j) {  // Start from i+1 to avoid duplicate checks
			auto c = colliders_[i]->check_intersection(colliders_[j]);
			if (c.collision) {
				std::cout << "Collision detected between object " << i 
						  << " and object " << j << "at: " << glm::to_string(c.collision_point) <<std::endl;
			}
		}
	}
	
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
	for (auto rigid_body : rigid_bodies_)
	{
		if (!rigid_body->is_fixed)
		{
			rigid_body->step(delta_t);
			rigid_body->clear_force();
		}
	}
	

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

void PhysicsEngine::register_rigid_body(rigid_body* rigid_body)
{
	this->rigid_bodies_.push_back(rigid_body);
}

void PhysicsEngine::register_collider(collider_modifier* collider)
{
	colliders_.push_back(collider);
}
