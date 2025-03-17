#include "PhysicsEngine.h"

#include <geometric.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../Scene/Object3D.h"
#include "../Scene/Scene.h"
#include "IntegrationMethods/Integrator.h"

void PhysicsEngine::evaluate_physics_step(double delta_t)
{
	auto overlaps = scene_->generate_overlaps_in_channel(PHYSICS);

	for (auto c : overlaps)
	{
		auto rigid_body_a = c.collider_a->associated_rigid_body;
		auto rigid_body_b = c.collider_b->associated_rigid_body;


		if (rigid_body_a && rigid_body_b && (!rigid_body_a->is_fixed || !rigid_body_b->is_fixed))
		{
			glm::vec3 a_center_collision_ws = c.collision.collision_point - c.collider_a->get_collider_center_ws();
			glm::vec3 b_center_collision_ws = c.collision.collision_point - c.collider_b->get_collider_center_ws();

			auto debug_tools = rigid_body_a->get_parent()->get_scene()->get_debug_tools();
			debug_tools->draw_debug_line(c.collision.collision_point, c.collider_a->get_collider_center_ws(),
			                             glm::vec3(1, 0, 0),
			                             20);
			debug_tools->draw_debug_line(c.collision.collision_point, c.collider_b->get_collider_center_ws(),
			                             glm::vec3(0, 1, 0),
			                             20);


			glm::vec3 v_rel_a = rigid_body_a->get_velocity() + cross(
				rigid_body_a->get_angular_velocity(), a_center_collision_ws);
			glm::vec3 v_rel_b = rigid_body_b->get_velocity() + cross(
				rigid_body_b->get_angular_velocity(), b_center_collision_ws);

			glm::vec3 v_rel = v_rel_a - v_rel_b;

			float avg_bounciness = (c.collider_a->associated_rigid_body->bounciness + c.collider_b->
				associated_rigid_body->bounciness) * 0.5;

			if (dot(v_rel, c.collision.collision_normal) < 0)
			{
				//collision
				float J = (-(1 + avg_bounciness) * dot(v_rel, c.collision.collision_normal)) /
					(1 / rigid_body_a->mass) + (1 / rigid_body_b->mass) + dot(
						cross(rigid_body_a->get_inverse_inertia_tensor_world_space() * cross(
							      a_center_collision_ws, c.collision.collision_normal), a_center_collision_ws) +
						cross(rigid_body_a->get_inverse_inertia_tensor_world_space() * cross(
							      b_center_collision_ws, c.collision.collision_normal), b_center_collision_ws)
						, c.collision.collision_normal);

				rigid_body_a->current_linear_impulse += J * (c.collision.collision_normal / rigid_body_a->mass);
				rigid_body_b->current_linear_impulse -= J * (c.collision.collision_normal / rigid_body_b->mass);

				rigid_body_a->current_angular_impulse += cross(
					a_center_collision_ws, J * c.collision.collision_normal);
				rigid_body_b->current_angular_impulse -= cross(
					b_center_collision_ws, J * c.collision.collision_normal);


				scene_->get_global_context()->logger->print_info(std::format("J = {}\n", J));
				//printf("J = %f\n", J);
			}
			//glm::vec3 vec_a_collision_point = 
		}
	}


	//calculate spring forces
	for (auto spring : springs_)
	{
		auto pos_1 = spring->point_1->get_parent()->getWorldPosition();
		auto pos_2 = spring->point_2->get_parent()->getWorldPosition();
		auto l = distance(pos_1, pos_2);
		auto force_direction = (pos_1 - pos_2) / l;
		auto f = -spring->stiffness * (l - spring->initial_length);
		spring->point_1->add_force(force_direction * f);
		spring->point_2->add_force(-force_direction * f);
	}

	//evaluate rigid body
	for (auto rigid_body : rigid_bodies_)
	{
		if (!rigid_body->is_fixed)
		{
			rigid_body->step(delta_t);
			rigid_body->clear_force();
			rigid_body->clear_impulses();
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
