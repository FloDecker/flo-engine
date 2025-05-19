#pragma once
#include "../Object3D.h"
#include "../Scene.h"

struct surfel_octree_element
{
	uint32_t surfels_at_layer_amount; // amount of surfels at this layer
	uint32_t surfels_at_layer_pointer;
	uint32_t next_layer_surfels_pointer[8];
};

class SurfelManagerOctree
{
public:
	SurfelManagerOctree(Scene* scene);
	void draw_ui();

	
	unsigned int samples_per_meter = 2;
	float surface_attachment_radius = 1.0f;
	int gi_primary_rays = 10;

	float total_extension = 512.0;
	int octree_levels = 8;

	int get_octree_level_for_surfel(const surfel* surfel);
	bool insert_surfel_into_octree(const surfel *surfel);
	
	void snap_samples_to_closest_surface();
	std::vector<surfel> samples();
	void add_surfel_uniforms_to_shader(ShaderProgram* shader) const; 
	void recalculate_surfels();

private:
	Scene* scene_;
	bool has_surfels_buffer_ = false;

	//cpu representation of ocree:
	surfel_octree_element* octree_;
	
	float SURFELS_BUCKET_SIZE = 1.0f; //in ws units
	unsigned int SURFELS_GRID_SIZE = 128; //actual size is SURFELS_BUCKET_SIZE * SURFELS_GRID_SIZE
	unsigned int SURFEL_BUFFER_AMOUNT = 1000000;
	
	texture_buffer_object* surfels_texture_buffer_positions_;
	texture_buffer_object* surfels_texture_buffer_normals_;
	texture_buffer_object* surfels_texture_buffer_color_;
	texture_buffer_object* surfels_texture_buffer_radii_;
	texture_buffer_object* surfels_uniform_grid;
	
	std::vector<surfel> samples_ = std::vector<surfel>();
	void clear_samples();
	bool draw_debug_tools_ = false;
	float points_per_square_meter = 1;
	
	void init_surfels_buffer();
	unsigned int get_surfel_buckets_from_ws_pos(glm::vec3 ws_pos, glm::vec3 ws_normal, unsigned int buckets[]);
	glm::vec3 get_surfel_bucket_center(glm::vec3 ws_pos) const;
	unsigned int get_surfel_bucket_from_ws_pos(glm::vec3 ws_pos) const;
};
