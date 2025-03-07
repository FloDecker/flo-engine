#pragma once
#include <set>

#include "../../../../../Core/CommonDataStructures/collision_channel.h"
#include "../../../../../Core/CommonDataStructures/struct_intersection.h"
#include "../../../../../Core/CommonDataStructures/ray_cast_result.h"
#include "../../../../../Util/RayIntersectionHelper.h"

class mesh_collider;
class rigid_body;
class box_collider;
class PhysicsEngine;

class collider_modifier : public modifier
{
public:
	explicit collider_modifier(Object3D* parent_game_object_3d);
	rigid_body* associated_rigid_body;
	int get_id() override;
	static int MODIFIER_ID;
	
	bool debug_draw_axis_aligned_bounding_box = false;
	void on_parent_recalculate_transforms() override;
	void on_parent_draw() override;
	
	virtual struct_intersection check_intersection(collider_modifier* other) = 0;
	virtual struct_intersection check_intersection_with(box_collider* box) = 0;
	virtual struct_intersection check_intersection_with(mesh_collider* mesh) = 0;
	
	virtual void is_in_proximity(glm::vec3 center_ws, float radius, ray_cast_result* result) = 0;
	ray_cast_result *is_in_proximity(glm::vec3 center_ws, float radius);
	
	virtual glm::vec3 get_center_of_mass_local() = 0;

	
	glm::vec3 get_collider_center_ws() const;

	void calculate_world_space_bounding_box();

	void ray_intersection_world_space(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
	                                          bool ignore_back_face,
	                                          ray_cast_result* ray_cast_result_out);
	
	virtual void ray_intersection_local_space(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
											  bool ignore_back_face,
											  ray_cast_result* ray_cast_result_out) = 0;

	rigid_body* get_associated_rigid_body() const;

	//collision channels
	std::set<collision_channel> collision_channels;


	StructBoundingBox* get_world_space_bounding_box();
	
protected:
	virtual StructBoundingBox calculate_world_space_bounding_box_internal_() = 0;
	StructBoundingBox current_world_space_bounding_box_;

};
