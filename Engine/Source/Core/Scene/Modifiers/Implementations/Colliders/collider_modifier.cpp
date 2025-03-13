#include "collider_modifier.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"
#include "../../../Object3D.h"
#include "../../../Scene.h"
#include "../../../../../Util/BoundingBoxHelper.h"

int collider_modifier::MODIFIER_ID = 100;

void collider_modifier::draw_gui()
{
	ImGui::Checkbox("Draw axis aligned debug bb",&this->debug_draw_axis_aligned_bounding_box);
	if (debug_draw_axis_aligned_bounding_box)
	{
		get_parent()->get_scene()->get_debug_tools()->draw_debug_cube(get_world_space_bounding_box() );
	}
}

collider_modifier::collider_modifier(Object3D* parent_game_object_3d, collision_channel default_channel): modifier(parent_game_object_3d)
{
	add_collision_channel(default_channel);
	parent_game_object_3d->get_scene()->register_collider(this);
	//search if the parent objects has rigid body modifier
	
	auto parent_collider = parent_game_object_3d->get_modifiers_by_id(10);
	for (auto modifier : parent_collider)
	{
		rigid_body* r = dynamic_cast<rigid_body*>(modifier);
		if (r == nullptr)
		{
			std::cerr << "tried casting modifier with id 10 but couldn't cast object to rigid body modifier\n";
		} else
		{
			r->collider = this;
			associated_rigid_body = r;
		}
	}	
}


int collider_modifier::get_id()
{
	return MODIFIER_ID;
}

void collider_modifier::on_parent_recalculate_transforms()
{
	calculate_world_space_bounding_box();
}

void collider_modifier::on_parent_draw()
{
}

ray_cast_result* collider_modifier::is_in_proximity(glm::vec3 center_ws, float radius)
{
	const auto result = new ray_cast_result();
	is_in_proximity(center_ws, radius, result);
	return result;
}

glm::vec3 collider_modifier::get_collider_center_ws() const
{
	return BoundingBoxHelper::get_center_of_bb(&current_world_space_bounding_box_);
}

void collider_modifier::calculate_world_space_bounding_box()
{
	current_world_space_bounding_box_ = calculate_world_space_bounding_box_internal_();
}

void collider_modifier::ray_intersection_world_space(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws,
                                                     float ray_length, bool ignore_back_face,
                                                     ray_cast_result* ray_cast_result_out)
{
	ray_intersection_local_space(parent->transform_position_to_local_space(ray_origin_ws),
	                             parent->transform_vector_to_local_space(ray_direction_ws), ray_length,
	                             ignore_back_face,
	                             ray_cast_result_out);

	ray_cast_result_out->hit_world_space = parent->transform_vertex_to_world_space(ray_cast_result_out->hit_local);
	ray_cast_result_out->hit_normal_world_space = parent->transform_vector_to_world_space(ray_cast_result_out->hit_normal_local);
	ray_cast_result_out->distance_from_origin = glm::distance(ray_origin_ws, ray_cast_result_out->hit_world_space);
}

rigid_body* collider_modifier::get_associated_rigid_body() const
{
	if (associated_rigid_body == nullptr)
	{
		std::cerr << "no rigid body associated with this collider \n";
		return nullptr;
	}
	return associated_rigid_body;
}


std::set<collision_channel> &collider_modifier::collision_channels()
{
	return collision_channels_;
}

bool collider_modifier::remove_collision_channel(collision_channel channel)
{
	if (collision_channels_.contains(channel))
	{
		collision_channels_.erase(channel);
		return true;
	}
	return false;
}

void collider_modifier::add_collision_channel(collision_channel channel)
{
	if (!collision_channels_.contains(channel))
	{
		collision_channels_.insert(channel);
	}

}


StructBoundingBox* collider_modifier::get_world_space_bounding_box()
{
	if (current_world_space_bounding_box_.max == glm::vec3() && current_world_space_bounding_box_.min == glm::vec3())
	{
		calculate_world_space_bounding_box();
	}
	return &current_world_space_bounding_box_;
}
