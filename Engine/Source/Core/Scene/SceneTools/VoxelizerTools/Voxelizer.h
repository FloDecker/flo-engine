#pragma once
#include <vector>

#include "AbstractVoxelizer.h"
#include "../../Object3D.h"
#include "../../Scene.h"
#include <glm.hpp>
#include "../../RayCast.h"
#include "../../../Scene/DebugPrimitives/Cube3D.h"

class Voxelizer : public AbstractVoxelizer
{
public:
	static glm::i16vec3 cubic_expansion_directions[26];


	explicit Voxelizer(Object3D* parent);
	void recalculate() override;
	void load_into_voxel_texture(texture_3d* texture_3d) override;
	void load_into_voxel_texture_df(texture_3d* texture_3d);
	bool create_level_set_matrix();
	bool reset_level_set_matrix();
	int get_length_of_level_set_matrix() const;
	bool is_level_set_matrix_initialized() const;
	void insert_into_level_set_matrix(glm::i16vec3 pos, int i) const;
	int get_value_of_level_set_matrix_at(glm::i16vec3 pos) const;
	bool is_in_level_set_matrix(glm::i16vec3 pos) const;
	glm::i16vec3 get_dimensions_of_level_set_matrix() const;

	glm::vec3 get_ws_pos_from_voxel_pos(glm::i16vec3 pos) const;
	glm::i16vec3 get_level_set_matrix_pos_from_ws(const glm::vec3& ws_pos);

	int get_unique_pos_id_in_matrix(glm::i16vec3 p) const;

private:
	int* level_set_matrix = nullptr;
	bool is_level_set_matrix_initialized_ = false;
	glm::i16vec3 level_set_matrix_dimensions; //TODO: this can be calculated without initializing the level set matrix
	std::vector<glm::vec3> voxel_positions_; //contains the coordinates of the zero level set in ws coordiantes 
	std::vector<glm::i16vec3> zero_level_set; //contains the index of the zero level set in


	//takes a voxel position and 3 vertices that define a triangle in world space, then it recursively expands the zero level set
	//on the triangle surface
	void expand_polygon_to_zero_level_set(collider_modifier* collider, unsigned int vertex_id_0,
	                                      struct_vertex_array* vertex_array,
	                                      glm::i16vec3 voxel_to_expand_from,
	                                      float radius);


	//takes the mesh collider from scene context and extracts the triangles that are contained in the bounding box
	//for every triangle that meets this condition the expand_polygon_to_zero_level_set is executed for it
	void calculate_area_filled_by_polygons(Scene* scene_context);
	void calculate_area_filled_recursive(Scene* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
	                                     glm::i16vec3 voxel_upper_right, glm::i16vec3 voxel_lower_left);
};
