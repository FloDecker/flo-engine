#pragma once
#include "collider_modifier.h"
#include "../../../../../Util/BoundingBoxHelper.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"

class box_collider final : public collider_modifier
{
public:
	explicit box_collider(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine, glm::vec3 max, glm::vec3 min)
	: collider_modifier(parent_game_object_3d ,physics_engine)
	{
		bounding_box.max = max;
		bounding_box.min = min;
		
	}
	
	explicit box_collider(Object3D* parent_game_object_3d, PhysicsEngine* physics_engine)
		: box_collider(parent_game_object_3d, physics_engine, glm::vec3(1.0f), glm::vec3(-1.0f))
	{
	}

	StructBoundingBox bounding_box;

	void set_box(glm::vec3 max, glm::vec3 min);


	bool check_intersection(collider_modifier* other) override;
	bool check_intersection_with(box_collider* box) override;

private:
	

};
