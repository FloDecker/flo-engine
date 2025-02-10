#include "rigid_body.h"
#include "../../Core/Scene/Collider.h"
#include "../../Object3D.h"
#include "../../../../Util/math_util.h"
#include "../../../PhysicsEngine/PhysicsEngine.h"

rigid_body::rigid_body(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine, MeshCollider* collider)
	: physics_object_modifier(parent_game_object_3d, physics_engine)
{
	physics_engine->register_rigid_body(this);
	collider_ = collider;
	pos_center_of_mass_object_space_ = collider_->get_center_of_mass();
	pos_center_of_mass_object_space_initial_ = pos_center_of_mass_object_space_;
	for (unsigned int i = 0; i < collider->get_vertex_arrays()->size(); i++)
	{
		auto vertex_array = collider->get_vertex_arrays()->at(i);
		for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
		{
			distances_to_center_of_mass_.push_back(
				vertex_array->vertices->at(j).position - pos_center_of_mass_object_space_);
			force_on_vertices_.push_back({0, 0, 0});
		}
	}


	inverse_inertia_tensor_object_space_ = inverse(collider_->get_inertia_tensor());
	angular_momentum_ = glm::vec3(0, 0, 0);
	update_inverse_inertia_tensor_world_space();
	update_angular_velocity();
}

void rigid_body::calculate_torque_and_force()
{
	torque_ = glm::vec3(0, 0, 0);
	for (unsigned int i = 0; i < force_on_vertices_.size(); i++)
	{
		auto force_at_vertex = force_on_vertices_.at(i);
		torque_ += cross(distances_to_center_of_mass_.at(i), force_at_vertex);
	}
}

void rigid_body::update_inverse_inertia_tensor_world_space()
{
	auto rotation_parent = get_parent()->get_global_rotation_matrix();
	inverse_inertia_tensor_world_space_ = rotation_parent * inverse_inertia_tensor_object_space_ * transpose(
		rotation_parent);
}

void rigid_body::update_angular_velocity()
{
	angular_velocity_ = inverse_inertia_tensor_world_space_ * angular_momentum_;
}

void rigid_body::step(float delta)
{
	calculate_torque_and_force();
	pos_center_of_mass_object_space_ += delta * velocity_center_of_mass_object_space_; //x_cm = x_cm + h * v_cm
	velocity_center_of_mass_object_space_ += delta * force_ / mass; //v_cm = v_cm + h * F/M

	//
	// //calculate rotation
	auto parent_rot = parent->get_quaternion_rotation();
	auto rot = parent_rot + (delta / 2) * (glm::quat(0, angular_velocity_.x, angular_velocity_.y, angular_velocity_.z) *
		parent_rot);
	parent->setRotationLocal(rot);
	
	angular_momentum_ += delta * torque_;
	
	update_inverse_inertia_tensor_world_space();
	update_angular_velocity();
	
	//printf("velocity center of mass: %f at force: %f | angular momentum: %f,%f,%f \n" ,glm::length(velocity_center_of_mass_object_space_), glm::length(force_),
		//angular_momentum_.x,angular_momentum_.y,angular_momentum_.z);
	
	auto r = parent->get_global_rotation_matrix();
	
	parent->move_local(delta * velocity_center_of_mass_object_space_);
	force_ = glm::vec3(0, 0, 0);
}

void rigid_body::apply_force_at_vertex(unsigned int vertex_id, glm::vec3 force)
{
	force_on_vertices_.at(vertex_id) += force;
}

void rigid_body::apply_force_ws(glm::vec3 direction_ws, glm::vec3 pos_ws, float force)
{
	force_ += normalize(direction_ws) * force;

	auto direction_local_space = normalize(parent->transform_vector_to_local_space(direction_ws));
	auto pos_local_space = parent->transform_position_to_local_space(pos_ws);
	auto cast_hit = ray_cast_hit();
	this->collider_->check_collision_ls(pos_local_space, direction_local_space, 100000.0, true, &cast_hit);
	if (!cast_hit.hit)
	{
		return;
	}
	
	glm::vec3 pos_object_space_0 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[0])+pos_center_of_mass_object_space_initial_;
	glm::vec3 pos_object_space_1 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[1])+pos_center_of_mass_object_space_initial_;
	glm::vec3 pos_object_space_2 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[2])+pos_center_of_mass_object_space_initial_;
	

	auto distribution = math_util::barycentric(cast_hit.hit_local, pos_object_space_0, pos_object_space_1, pos_object_space_2);
	

	apply_force_at_vertex(cast_hit.vertex_indices[0], direction_local_space * (force * distribution.x));
	apply_force_at_vertex(cast_hit.vertex_indices[1], direction_local_space * (force * distribution.y));
	apply_force_at_vertex(cast_hit.vertex_indices[2], direction_local_space * (force * distribution.z));

	
}

void rigid_body::clear_force()
{
	physics_object_modifier::clear_force();
	for (int i = 0; i < force_on_vertices_.size(); i++)
	{
		force_on_vertices_.at(i) = glm::vec3(0, 0, 0);
		
	}
}
