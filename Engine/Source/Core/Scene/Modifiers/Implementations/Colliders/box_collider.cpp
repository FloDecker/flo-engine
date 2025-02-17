#include "box_collider.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../../../Object3D.h"

glm::vec3 box_collider::get_collider_center_ws()
{
	return parent->transform_vertex_to_world_space(BoundingBoxHelper::get_center_of_bb(&bounding_box));
}

void box_collider::set_box(glm::vec3 max, glm::vec3 min)
{
	bounding_box.max = max;
	bounding_box.min = min;
}

struct_collision box_collider::check_intersection(collider_modifier* other)
{
	return other->check_intersection_with(this);
}

struct_collision box_collider::check_intersection_with(box_collider* box)
{
	struct_collision c =  BoundingBoxHelper::are_intersecting(&this->bounding_box, &box->bounding_box,
	                                           this->parent->getGlobalTransform(), box->parent->getGlobalTransform());
	if (c.collision)
	{
		std::printf("Intersection local: %s \n", glm::to_string(c.collision_point).c_str());

		c.collision_normal = box->parent->transform_vector_to_world_space(c.collision_normal);
		//c.collision_point = box->parent->transform_vertex_to_world_space(c.collision_point);
	}
	
	return c;
}
