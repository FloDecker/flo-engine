#pragma once
#include <glm.hpp>

#include "Texture.h"

class texture_3d : public texture
{
	//pointer to the voxel data
	//Memory Alignment : [(4)R , (4)G , (4) B, (4) A]
	unsigned short int* _data;

	//amount of voxels in every direction
	unsigned int width_ = 0;
	unsigned int height_ = 0;
	unsigned int depth_ = 0;

	//amount of voxels in one world space unit
	int step_size_ = 1;

	//position of voxel field edges in world space coordinates
	glm::vec3 voxel_field_lower_left_ = glm::vec3(0.0);
	glm::vec3 voxel_field_upper_right_ = glm::vec3(0.0);

public:
	void initialize();
	unsigned int getTexture();
	void initalize_as_voxel_data(glm::vec3 voxel_field_lower_left,
	                             glm::vec3 voxel_field_upper_right,
	                             int steps_per_world_space_unit);
	void write_to_voxel_field(unsigned short r, unsigned short g, unsigned short b, unsigned short a,
	                          unsigned int pos_width, unsigned int pos_height, unsigned int pos_depth);
	void write_to_voxel_field_float(unsigned short r, unsigned short g, unsigned short b, unsigned short a,
	                                float pos_width, float pos_height, float pos_depth);
	unsigned short get_distance_at(unsigned int pos_width, unsigned int pos_height, unsigned int pos_depth);


	//getter
	unsigned int get_width() const { return width_; }
	unsigned int get_height() const { return height_; }
	unsigned int get_depth() const { return depth_; }
	float get_step_size() const { return step_size_; }
	glm::vec3 get_voxel_field_lower_left() const { return voxel_field_lower_left_; }
	glm::vec3 get_voxel_field_upper_right() const { return voxel_field_upper_right_; }
};
