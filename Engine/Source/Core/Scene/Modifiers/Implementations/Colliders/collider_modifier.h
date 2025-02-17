#pragma once
#include "../physics_object_modifier.h"
#include "../../../../../Core/CommonDataStructures/struct_collision.h"

class rigid_body;
class box_collider;
class PhysicsEngine;

class collider_modifier : public modifier
{
public:
	explicit collider_modifier(Object3D* parent_game_object_3d,PhysicsEngine* physics_engine);
	rigid_body* associated_rigid_body;
	int get_id() override;
	virtual struct_collision check_intersection(collider_modifier* other) = 0;
	virtual struct_collision check_intersection_with(box_collider* box) = 0;
	virtual glm::vec3 get_collider_center_ws() = 0;

	rigid_body* get_associated_rigid_body() const;

};
