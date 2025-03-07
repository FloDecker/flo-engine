#include "box_collider.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../../../Object3D.h"

void box_collider::ray_intersection_local_space(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
                                                bool ignore_back_face, ray_cast_result* ray_cast_result_out)
{
	std::cerr<<"box_collider::ray_intersection_local_space not implemented\n "<<std::endl;
	
}

void box_collider::set_box(glm::vec3 max, glm::vec3 min)
{
	bounding_box.max = max;
	bounding_box.min = min;
}


struct_intersection box_collider::check_intersection(collider_modifier* other)
{
	return other->check_intersection_with(this);
}


struct_intersection box_collider::check_intersection_with(box_collider* box)
{
	struct_intersection c =  BoundingBoxHelper::are_intersecting(&this->bounding_box, &box->bounding_box,
	                                           this->parent->getGlobalTransform(), box->parent->getGlobalTransform());
	if (c.intersected)
	{
		std::printf("Intersection local: %s \n", glm::to_string(c.collision_point).c_str());

		c.collision_normal = box->parent->transform_vector_to_world_space(c.collision_normal);
	}
	
	return c;
}

struct_intersection box_collider::check_intersection_with(mesh_collider* mesh)
{
	std::cerr << "function not implemented box_collider::check_intersection_with\n";
	return struct_intersection();
}

glm::vec3 box_collider::get_center_of_mass_local()
{
	return BoundingBoxHelper::get_center_of_bb(&bounding_box);
}


void box_collider::is_in_proximity(glm::vec3 center_ws, float radius, ray_cast_result* result)
{
}


StructBoundingBox box_collider::calculate_world_space_bounding_box_internal_()
{
	StructBoundingBox result;
	BoundingBoxHelper::transform_local_bb_to_world_space_axis_aligned(&result, &bounding_box, this->parent->getGlobalTransform());
	return result;
}
