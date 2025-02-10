#include "box_collider.h"

#include "../../../Object3D.h"

void box_collider::set_box(glm::vec3 max, glm::vec3 min)
{
	bounding_box.max = max;
	bounding_box.min = min;
}

bool box_collider::check_intersection(collider_modifier* other)
{
	return other->check_intersection_with(this);
}

bool box_collider::check_intersection_with(box_collider* box)
{
	//the coord system of the incoming object is the reference frame (B) this is (A)

	//transform to local space of b
	
	//local space A -> World space
	glm::vec3 a_max = this->bounding_box.max;
	glm::vec3 a_min = this->bounding_box.min;

	a_max = this->parent->transform_vertex_to_world_space(a_max);
	a_min = this->parent->transform_vertex_to_world_space(a_min);

	// World space -> local space B

	a_max = box->parent->transform_position_to_local_space(a_max);
	a_min = box->parent->transform_position_to_local_space(a_min);

	std::printf("check A: %s against B: %s\n", this->parent->name.c_str(), box->parent->name.c_str());

	return true;

	
}
