#pragma once
#include "collider_modifier.h"
#include "../../../../../Util/BoundingBoxHelper.h"
#include "../../../../PhysicsEngine/PhysicsEngine.h"
struct ray_cast_result;

class box_collider final : public collider_modifier
{
public:
	explicit box_collider(Object3D* parent_game_object_3d, glm::vec3 max, glm::vec3 min)
	: collider_modifier(parent_game_object_3d)
	{
		bounding_box.max = max;
		bounding_box.min = min;
		
	}
	
	explicit box_collider(Object3D* parent_game_object_3d)
		: box_collider(parent_game_object_3d, glm::vec3(1.0f), glm::vec3(-1.0f))
	{
	}
	
	void ray_intersection_local_space(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
	                                  bool ignore_back_face, ray_cast_result* ray_cast_result_out) override;
	
	StructBoundingBox bounding_box;
	void set_box(glm::vec3 max, glm::vec3 min);


	struct_intersection check_intersection(collider_modifier* other) override;
	struct_intersection check_intersection_with(box_collider* box) override;
	void is_in_proximity(glm::vec3 center_ws, float radius, ray_cast_result* result) override;
	struct_intersection check_intersection_with(mesh_collider* mesh) override;
	glm::vec3 get_center_of_mass_local() override;
protected:
	StructBoundingBox calculate_world_space_bounding_box_internal_() override;
};
