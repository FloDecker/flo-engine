#include "collider_modifier.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"
#include "../../../Object3D.h"
#include "../../../Scene.h"
#include "../../../../../Util/BoundingBoxHelper.h"
#include "../../../DebugPrimitives/Cube3D.h"

int collider_modifier::MODIFIER_ID = 100;

collider_modifier::collider_modifier(Object3D* parent_game_object_3d): modifier(parent_game_object_3d)
{
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
	calculate_world_space_bounding_box_internal_();
}

void collider_modifier::on_parent_draw()
{
	if (debug_draw_axis_aligned_bounding_box)
	{
	}
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

StructBoundingBox* collider_modifier::get_world_space_bounding_box()
{
	return &current_world_space_bounding_box_;
}
