#pragma once
#include <set>

#include "../Object3D.h"
#include "../Scene.h"
#include "surfel.h"
#include "../../Renderer/ssbo.h"
#include <bitset>

#include "../../Renderer/ssbo_double_buffer.h"

constexpr int SURFEL_BUCKET_SIZE_ON_GPU = 128; //amount of surfels a single bucket holds (also the increment)
constexpr int SURFELS_BUCKET_AMOUNT = 40000; //total amount of surfel buckets that can be allocated
constexpr int SURFEL_BUCKET_TOTAL_MEMORY = SURFEL_BUCKET_SIZE_ON_GPU * SURFELS_BUCKET_AMOUNT; //total memory allocated for surfels 
constexpr int SURFEL_OCTREE_SIZE = 100000;


class Camera3D;

//this is mapped 1:1 to GPU memory 
struct surfel_octree_element
{
	// amount of surfels at this layer - the last 8 bits are reserved to indicated witch child octrees exist 
	uint32_t surfels_at_layer_amount;
	uint32_t surfels_at_layer_pointer; //
	uint32_t next_layer_surfels_pointer[8];
};


struct surfel_gpu
{
	glm::vec4 position_r = {}; //position + radius 
	glm::vec4 normal = {};
	glm::vec4 radiance_ambient = {}; //radiance_ambient(frist bounce) + sample amout
	glm::vec4 radiance_direct_and_surface = {};
};

struct surfel_allocation_metadata
{
	uint32_t surfel_bucket_pointer = 0; 
	uint32_t surfel_octree_pointer = 0;
	uint32_t debug_int_32 = 0;
};

//this holds additional metadata information of the buckets that is only needed on the CPU
struct surfel_octree_metadata
{
	int parent;
	int bucket_size = 0;
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
	int surfel_gi_updates_per_tick = 64;
	float surface_attachment_radius = 1.0f;
	int gi_primary_rays = 10;
	float illumination_derivative_threshold = 0.1f;
	float minimal_surfel_radius = 1.0f;
	bool update_surfels_next_tick = false;

	float octree_total_extension = 512.0;
	int octree_levels = 9;

	int get_octree_level_for_surfel(const surfel* surfel);
	bool insert_surfel_into_octree(surfel* surfel);
	void generate_surfels_via_compute_shader() const;
	void find_surfels_to_update();
	bool update_surfels_in_update_queue(int amount);
	void find_best_world_positions_to_update_lighting() const;
	void compute_shader_ao_approximation(uint32_t level, glm::uvec3 pos_in_octree) const;
	bool remove_surfel(const surfel* surfel);


	void generate_surfels();
	void recalculate_surfels();
	void update_surfels();
	surfel* get_closest_neighbour_on_level(const surfel* s) const;
	
	static glm::vec3 get_illumination_gradient(const surfel* s_1, const surfel* s_2);
	static surfel get_combining_surfel(const surfel* s_1, const surfel* s_2);
	bool merge_surfels(const surfel* s_1, const surfel* s_2, const surfel& new_surfel, std::set<surfel*>& additional_overlaps, float
	                   max_gradient_difference);
	bool insert_surfel(const surfel& surfel_to_insert);
	void register_scene_data(Camera3D *camera, const framebuffer_object* surfel_framebuffer);
	

	//compute shader
	compute_shader *insert_surfel_compute_shader;
	compute_shader* compute_shader_approxmiate_ao;
	compute_shader* compute_shader_find_least_shaded_pos;

	void tick();

	//copy the new data from the surfel compute buffer to the backbuffer of the consumer ssbo
	void copy_data_from_compute_to_back_buffer() const;
	void swap_surfel_buffers() const;

	int get_manager_state() const;

private:
	Scene* scene_;
	Camera3D *camera_ = nullptr; //camera object used to determine the center of view 
	bool has_surfels_buffer_ = false;
	uint64_t tick_amount_ = 0;

	uint64_t light_update_current_level_ = 0;
	uint64_t light_update_current_size_ = 0;
	uint64_t light_update_current_pos_ = 0;

	//cpu representation of ocree:
	surfel_octree_element* octree_;

	//additional octree management data that isn't needed on the GPU
	surfel_octree_metadata* octree_metadata_;


	//actual size is SURFEL_BUCKET_SIZE_ON_GPU * SURFEL_BUCKET_SIZE_ON_GPU


	unsigned int memory_limitation_count_bottom_size = 0;
	unsigned int memory_limitation_octree_size = 0;
	unsigned int memory_limitation_bucket_size = 0;

	std::bitset<SURFEL_BUCKET_TOTAL_MEMORY> allocation_bitmap;

	ssbo<surfel_gpu>* surfel_ssbo_producer_;
	ssbo<surfel_octree_element>* surfels_octree_producer_;

	ssbo_double_buffer<surfel_gpu>* surfel_ssbo_consumer_;
	ssbo_double_buffer<surfel_octree_element>* surfels_octree_consumer_;

	ssbo<surfel_allocation_metadata>* surfel_allocation_metadata;

	ssbo<glm::vec4>* least_shaded_regions;

	GLsync compute_fence_;
	

	//a vector that holds all surfels currently used
	std::vector<std::unique_ptr<surfel>> surfels_ = std::vector<std::unique_ptr<surfel>>();

	void clear_samples();
	bool draw_debug_tools_ = false;
	float points_per_square_meter = 1;
	float starting_radius = 1.0f;

	bool draw_debug_boxes_ = false;

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
	glm::uvec3 get_bucket_coordinates_from_ws(glm::vec3 ws_pos, int level) const;
	glm::vec3 get_center_of_sub_octree_level(int current_layer, glm::vec3 current_center, glm::vec<3, float> pos_relative) const;
	glm::vec3 get_ws_bucket_lowest_edge_from_octree_index(int layer, glm::uvec3 octree_index) const;
	glm::vec3 get_ws_bucket_center_from_octree_index(int layer, glm::uvec3 octree_index) const;
	static unsigned int get_bucket_amount_at_level(unsigned int level);
	float node_size_at_level(unsigned int level) const;
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
	bool create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array, uint32_t allocation_size);

	//uploads the surfel data to one specific location in GPU memory 
	bool upload_surfel_data(surfel* surfel_to_insert, unsigned int insertion_index) const;

	bool update_surfel_data(surfel* surfel_to_insert) const;
	int get_surfel_pos_in_bucket(unsigned int bucket_index, const surfel* surfel_to_find) const;

	void dump_surfel_octree_to_gpu_memory();
	void update_octree_data_on_gpu(unsigned int octree_index);
	void remove_surfel_from_bucket_on_gpu(unsigned int bucket_start, unsigned int index,
	                                      unsigned int last_bucket_element);

	void clear_surfels_on_gpu_() const;

	glm::vec2 get_surfel_illumination_gradient(surfel* s);
	void create_packed_circles(glm::vec3 center, glm::vec3 normal, float radius, glm::vec3 color);

	std::vector<std::pair<unsigned, std::array<unsigned, 3>>> sample_positons_ = {};

	/*current state
	//states:
	0 -> inserting surfels
	1 -> computing AO
	2 -> copying from compute buffer to backbuffer

	*/
	int manager_state_ = 0;
};
