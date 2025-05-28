#pragma once
#include "../Object3D.h"
#include "../Scene.h"
#include "surfel.h"

struct surfel_octree_element
{
	// amount of surfels at this layer - the last 8 bits are reserved to indicated witch child octrees exist 
	uint32_t surfels_at_layer_amount; 
	uint32_t surfels_at_layer_pointer; //
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
	int octree_levels = 9;

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
	unsigned int SURFELS_BOTTOM_LEVEL_SIZE = 40000; //actual size is SURFEL_BUCKET_SIZE_ON_GPU * SURFEL_BUCKET_SIZE_ON_GPU
	unsigned int SURFEL_OCTREE_SIZE = 100000;
	const uint32_t SURFEL_BUCKET_SIZE_ON_GPU = 10; //space amount allocated for the surfels an octree element points to 

	
	texture_buffer_object* surfels_texture_buffer_positions_;
	texture_buffer_object* surfels_texture_buffer_normals_;
	texture_buffer_object* surfels_texture_buffer_color_;
	texture_buffer_object* surfels_texture_buffer_radii_;
	texture_buffer_object* surfels_octree;
	
	std::vector<surfel> samples_ = std::vector<surfel>();
	void clear_samples();
	bool draw_debug_tools_ = false;
	float points_per_square_meter = 1;
	
	void init_surfels_buffer();

	static bool is_child_octree_bit_set_at_(const surfel_octree_element* surfel_octree_element, uint8_t pos);
	static void set_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, uint8_t pos);
	static void increment_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element);
	static unsigned int get_surfel_count_in_octree_element(const surfel_octree_element* surfel_octree_element);

	bool insert_surfel_into_octree_recursive(const surfel *surfel_to_insert, int current_layer, glm::vec3 current_center, int target_layer, int
	                                         current_octree_element_index);
	uint32_t next_free_spot_in_octree_ = 1;
	bool create_new_octree_element(uint32_t& index);

	//GPU calls

	//surfel stack
	//surfels are allocated on a stack on the gpu, an octree element that contains surfels points to a position on that stack
	//TODO: reallocation of surfel buckets that exceed the bucket size 

	uint32_t surfel_stack_pointer = 0;
	bool create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array);
	bool upload_surfel_data(const surfel *surfel_to_insert, const surfel_octree_element* surfel_octree_element) const;

	void dump_surfle_octree_to_gpu_memory_();

	
};
