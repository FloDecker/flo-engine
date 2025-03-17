#pragma once
#include "collider_modifier.h"
#include "../../Core/CommonDataStructures/StructVertexArray.h"

class Mesh;

class mesh_collider final : public collider_modifier
{
public:
	mesh_collider(Object3D* parent_game_object_3d, std::vector<struct_vertex_array*> vertex_arrays)
		: collider_modifier(parent_game_object_3d), vertex_arrays_(vertex_arrays)
	{
	}

protected:
	StructBoundingBox calculate_world_space_bounding_box_internal_() override;

public:
	mesh_collider(Object3D* parent_game_object_3d, Mesh* mesh);

	void ray_intersection_local_space(glm::vec3 ray_origin_ls, glm::vec3 ray_direction_ls, float ray_length,
	                                  bool ignore_back_face, ray_cast_result* ray_cast_result_out) override;

	struct_intersection check_intersection(collider_modifier* other) override;
	struct_intersection check_intersection_with(box_collider* box) override;
	void is_in_proximity(glm::vec3 center_ws, float radius, ray_cast_result* result) override;
	struct_intersection check_intersection_with(mesh_collider* mesh) override;
	glm::vec3 get_center_of_mass_local() override;
	void scatter_points_on_surface(std::vector<vertex>* points, unsigned amount) override;

private:
	void is_in_proximity_vertex_array(glm::vec3 center_ws, float radius, unsigned int vertex_array_id,
	                                  ray_cast_result* result) const;
	std::vector<struct_vertex_array*> vertex_arrays_;
};
