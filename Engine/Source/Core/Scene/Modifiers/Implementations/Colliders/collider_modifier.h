#pragma once
#include "../physics_object_modifier.h"
#include "../../../../../Core/CommonDataStructures/struct_collision.h"

class box_collider;
class PhysicsEngine;

class collider_modifier : public physics_object_modifier
{
public:
	explicit collider_modifier(Object3D* parent_game_object_3d,PhysicsEngine* physics_engine);

	virtual struct_collision check_intersection(collider_modifier* other) = 0;
	
	virtual struct_collision check_intersection_with(box_collider* box) = 0;
};
