#include "rigid_body.h"
#include "../../Object3D.h"
#include "../../../../Util/math_util.h"
#include "../../../PhysicsEngine/PhysicsEngine.h"

rigid_body::rigid_body(Object3D* parent_game_object_3d)
	: physics_object_modifier(parent_game_object_3d)
{
	physics_engine_->register_rigid_body(this);

	//search if the parent objects has collider modifiers 
	auto parent_collider = parent_game_object_3d->get_modifiers_by_id(100);
	for (auto modifier : parent_collider)
	{
		auto c = dynamic_cast<collider_modifier*>(modifier);
		if (c == nullptr)
		{
			std::cerr << "tried casting modifier with id 100 but couldn't cast object to collider modifier\n";
		}
		else
		{
			collider = c;
			c->associated_rigid_body = this;
		}
	}

	//pos_center_of_mass_object_space_ = collider->get_center_of_mass_local();
	//pos_center_of_mass_object_space_initial_ = pos_center_of_mass_object_space_;

	//TODO: fix this: 
	// for (unsigned int i = 0; i < collider->get_vertex_arrays()->size(); i++)
	// {
	// 	auto vertex_array = collider->get_vertex_arrays()->at(i);
	// 	for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
	// 	{
	// 		distances_to_center_of_mass_.push_back(
	// 			vertex_array->vertices->at(j).position - pos_center_of_mass_object_space_);
	// 		force_on_vertices_.push_back({0, 0, 0});
	// 	}
	// }


	//TODO : fix this 
	//inverse_inertia_tensor_object_space_ = inverse(collider_->get_inertia_tensor());
	angular_momentum_ = glm::vec3(0, 0, 0);
	update_inverse_inertia_tensor_world_space();
	update_angular_velocity();
}

int rigid_body::get_id()
{
	return 10;
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
	if (skip_next_step)
	{
		skip_next_step = false;
		return;
	}
	calculate_torque_and_force();
	pos_center_of_mass_object_space_ += delta * velocity_center_of_mass_object_space_; //x_cm = x_cm + h * v_cm
	acceleration_ = force_ / mass + (gravity_enabled ? physics_constants::gravity_vector : glm::vec3());
	velocity_center_of_mass_object_space_ += delta * acceleration_; //v_cm = v_cm + h * F/M

	velocity_center_of_mass_object_space_ += current_linear_impulse; //add linear impulse to current velocity


	//calculate rotation
	auto parent_rot = parent->get_quaternion_rotation();
	auto rot = parent_rot + (delta / 2) * (glm::quat(0, angular_velocity_.x, angular_velocity_.y, angular_velocity_.z) *
		parent_rot);
	parent->setRotationLocal(rot);

	angular_momentum_ += delta * torque_;
	angular_momentum_ += current_angular_impulse;

	update_inverse_inertia_tensor_world_space();
	update_angular_velocity();

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
	auto cast_hit = ray_cast_result();
	this->collider->ray_intersection_local_space(pos_local_space, direction_local_space, 100000.0, true, &cast_hit);
	if (!cast_hit.hit)
	{
		return;
	}

	glm::vec3 pos_object_space_0 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[0]) +
		pos_center_of_mass_object_space_initial_;
	glm::vec3 pos_object_space_1 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[1]) +
		pos_center_of_mass_object_space_initial_;
	glm::vec3 pos_object_space_2 = distances_to_center_of_mass_.at(cast_hit.vertex_indices[2]) +
		pos_center_of_mass_object_space_initial_;


	auto distribution = math_util::barycentric(cast_hit.hit_local, pos_object_space_0, pos_object_space_1,
	                                           pos_object_space_2);


	apply_force_at_vertex(cast_hit.vertex_indices[0], direction_local_space * (force * distribution.x));
	apply_force_at_vertex(cast_hit.vertex_indices[1], direction_local_space * (force * distribution.y));
	apply_force_at_vertex(cast_hit.vertex_indices[2], direction_local_space * (force * distribution.z));
}

void rigid_body::recalculate_inertia(unsigned int index_of_collider)
{
	//
	// auto c = colliders.;
	// for (auto c : colliders)
	// {
	// 	glm::mat3 MeshCollider::calculate_inertia_tensor_internal()
	// 	{
	// 		auto inertia_tensor = glm::mat3(0);
	// 		auto center_of_mass = get_center_of_mass();
	// 		for (unsigned int i = 0; i < vertex_arrays_.size(); i++)
	// 		{
	// 			auto vertex_array = this->vertex_arrays_.at(i);
	// 			for (unsigned int j = 0; j < vertex_array->vertices->size(); j++)
	// 			{
	// 				vertex v = vertex_array->vertices->at(j);
	// 				auto vec_to_point = v.position - center_of_mass;
	// 				inertia_tensor += outerProduct(vec_to_point, vec_to_point);
	// 			}
	// 		}
	// 		return inertia_tensor;
	// 	}
	// }
}

void rigid_body::clear_force()
{
	physics_object_modifier::clear_force();
	for (int i = 0; i < force_on_vertices_.size(); i++)
	{
		force_on_vertices_.at(i) = glm::vec3(0, 0, 0);
	}
}

void rigid_body::clear_impulses()
{
	current_linear_impulse = glm::vec3(0, 0, 0);
	current_angular_impulse = glm::vec3(0, 0, 0);
}

void rigid_body::set_velocity(glm::vec3 velocity)
{
	velocity_center_of_mass_object_space_ = parent->transform_vector_to_local_space(velocity);
}

void rigid_body::set_angular_momentum(glm::vec3 angular_momentum)
{
	angular_momentum_ = angular_momentum;
}

glm::vec3 rigid_body::get_angular_velocity() const
{
	return angular_velocity_;
}

glm::mat3 rigid_body::get_inverse_inertia_tensor_world_space() const
{
	return inverse_inertia_tensor_world_space_;
}

glm::vec3 rigid_body::get_velocity() const
{
	return velocity_center_of_mass_object_space_;
}
