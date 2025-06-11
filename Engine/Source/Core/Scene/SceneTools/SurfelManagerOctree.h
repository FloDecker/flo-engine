#pragma once
#include <set>

#include "../Object3D.h"
#include "../Scene.h"
#include "surfel.h"
#include <thread>


//this is mapped 1:1 to GPU memory 
struct surfel_octree_element
{
	// amount of surfels at this layer - the last 8 bits are reserved to indicated witch child octrees exist 
	uint32_t surfels_at_layer_amount;
	uint32_t surfels_at_layer_pointer; //
	uint32_t next_layer_surfels_pointer[8];
};

//this holds additional metadata information of the buckets that is only needed on the CPU
struct surfel_octree_metadata
{
	int parent;
	//represents the surfels associated with this bucket
	//-> they are in the same order as on the GPU 
	std::vector<surfel*>* surfels = new std::vector<surfel*>;
};

class SurfelManagerOctree
{
public:
	SurfelManagerOctree(Scene* scene);
	void draw_ui();

	unsigned int samples_per_meter = 2;
	int update_surfels_per_tick = 1;
	float surface_attachment_radius = 1.0f;
	int gi_primary_rays = 10;

	float total_extension = 512.0;
	int octree_levels = 9;

	int get_octree_level_for_surfel(const surfel* surfel);
	bool insert_surfel_into_octree(surfel* surfel);
	bool remove_surfel(const surfel* surfel);


	void generate_surfels();
	void add_surfel_uniforms_to_shader(ShaderProgram* shader) const;
	void recalculate_surfels();
	void update_surfels();
	surfel* get_closest_neighbour_on_level(const surfel* s) const;
	
	static glm::vec3 get_illumination_gradient(const surfel* s_1, const surfel* s_2);
	static surfel get_combining_surfel(const surfel* s_1, const surfel* s_2);
	bool merge_surfels(const surfel* s_1, const surfel* s_2, const surfel& new_surfel, std::set<surfel*>& additional_overlaps, float
	                   max_gradient_difference);


private:
	Scene* scene_;
	bool has_surfels_buffer_ = false;

	//cpu representation of ocree:
	surfel_octree_element* octree_;

	//additional octree management data that isn't needed on the GPU
	surfel_octree_metadata* octree_metadata_;

	unsigned int SURFELS_BOTTOM_LEVEL_SIZE = 40000;
	//actual size is SURFEL_BUCKET_SIZE_ON_GPU * SURFEL_BUCKET_SIZE_ON_GPU
	unsigned int SURFEL_OCTREE_SIZE = 100000;
	const uint32_t SURFEL_BUCKET_SIZE_ON_GPU = 128; //space amount allocated for the surfels an octree element points to 
	const uint32_t MAX_SURFEL_BUCKET_SIZE_ON_GPU = 1024;

	unsigned int memory_limitation_count_bottom_size = 0;
	unsigned int memory_limitation_octree_size = 0;
	unsigned int memory_limitation_bucket_size = 0;

	texture_buffer_object* surfels_texture_buffer_positions_;
	texture_buffer_object* surfels_texture_buffer_normals_;
	texture_buffer_object* surfels_texture_buffer_color_;
	texture_buffer_object* surfels_texture_buffer_radii_;
	texture_buffer_object* surfels_octree;

	//a vector that holds all surfels currently used
	std::vector<std::unique_ptr<surfel>> surfels_ = std::vector<std::unique_ptr<surfel>>();

	void clear_samples();
	bool draw_debug_tools_ = false;
	float points_per_square_meter = 1;
	float starting_radius = 1.0f;

	void init_surfels_buffer();
	void reset_surfels_buffer();

	static bool is_child_octree_bit_set_at_(const surfel_octree_element* surfel_octree_element, uint8_t pos);
	static void set_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, uint8_t pos);
	static void unset_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, uint8_t pos);
	static bool are_all_child_octree_bits_empty_(surfel_octree_element* surfel_octree_element);

	static void increment_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element);
	static void decrement_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element);
	static unsigned int get_surfel_count_in_octree_element(const surfel_octree_element* surfel_octree_element);
	bool insert_surfel_in_surrounding_buckets(surfel* surfel, int target_layer);
	glm::vec3 get_surfel_bucket_center(glm::vec3 ws_pos, int level) const;
	glm::vec3 get_center_of_sub_octree_level(int current_layer, glm::vec3 current_center, glm::vec<3, float> pos_relative) const;
	bool insert_surfel_into_octree_recursive(surfel* surfel_to_insert, int current_layer, glm::vec3 current_center,
	                                         int target_layer, int
	                                         current_octree_element_index, bool insert_into_surroundings,
	                                         glm::vec3 target_pos);
	void get_surfels_in_radius_recursive(glm::vec3 ws_pos, float radius, int current_layer, glm::vec3 current_center, int current_octree_element_index, std::set
	                                     <surfel*>* found_surfels);
	void get_overlapping_octree_elements (glm::vec3 center, float radius, int layer, std::vector<glm::vec3>& result) const;
	uint32_t next_free_spot_in_octree_ = 1;
	bool create_new_octree_element(uint32_t& index, uint32_t parent_index);
	uint32_t last_updated_surfel = 0;
	//GPU calls

	//surfel stack
	//surfels are allocated on a stack on the gpu, an octree element that contains surfels points to a position on that stack
	//TODO: reallocation of surfel buckets that exceed the bucket size 

	uint32_t surfel_stack_pointer = 0;
	bool create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array);

	//uploads the surfel data to one specific location in GPU memory 
	bool upload_surfel_data(surfel* surfel_to_insert, unsigned int insertion_index) const;

	bool update_surfel_data(surfel* surfel_to_insert) const;
	void update_surfel_thread();
	int get_surfel_pos_in_bucket(unsigned int bucket_index, const surfel* surfel_to_find) const;

	void dump_surfel_octree_to_gpu_memory();
	void update_octree_data_on_gpu(unsigned int octree_index);
	void remove_surfel_from_bucket_on_gpu(unsigned int bucket_start, unsigned int index,
	                                      unsigned int last_bucket_element);
};
