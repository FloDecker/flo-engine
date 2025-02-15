#include "box_collider.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/string_cast.hpp>

#include "../../../Object3D.h"

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
	return BoundingBoxHelper::are_intersecting(&this->bounding_box, &box->bounding_box,
	                                           this->parent->getGlobalTransform(), box->parent->getGlobalTransform());
}
