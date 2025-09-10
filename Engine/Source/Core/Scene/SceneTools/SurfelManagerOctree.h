#pragma once
#include "../Object3D.h"
#include "../Scene.h"
#include "../../Renderer/ssbo.h"
#include "../../Renderer/ssbo_double_buffer.h"

constexpr int SURFEL_BUCKET_SIZE_ON_GPU = 256; //amount of surfels a single bucket holds (also the increment)
constexpr int SURFELS_BUCKET_AMOUNT = 40000; //total amount of surfel buckets that can be allocated
constexpr int SURFEL_BUCKET_TOTAL_MEMORY = SURFEL_BUCKET_SIZE_ON_GPU * SURFELS_BUCKET_AMOUNT;
//total memory allocated for surfels 
constexpr int SURFEL_OCTREE_SIZE = 100000;


class Camera3D;

struct surfel_octree_element
{
	uint32_t surfels_at_layer_amount;
	uint32_t surfels_at_layer_pointer; 
	uint32_t next_layer_surfels_pointer[8];
};

struct surfel_gpu
{
	glm::vec4 position_r = {};
	glm::vec4 normal = {};
	glm::vec4 albedo = {};
	glm::vec4 radiance_ambient = {};
	glm::vec4 radiance_direct_and_surface = {};
	uint32_t copy_locations[8];
};

struct surfel_allocation_metadata
{
	uint32_t surfel_bucket_pointer = 0;
	uint32_t surfel_octree_pointer = 0;
	uint32_t octree_pointer_update_index = 0;
	uint32_t debug_int_32 = 0;
};

struct state_history
{
	int state;
	double time_stamp;
	double render_time;
};

struct surfel_parameters
{
	float minimal_surfel_radius = 1.0f;
	float surfel_insertion_threshold = 0.4f;
	float surfel_insert_size_multiplier = 1.4f;
	int pixels_per_surfel = 128;
};

class SurfelManagerOctree
{
public:
	SurfelManagerOctree(Scene* scene);
	void draw_ui();

	int surfel_gi_updates_per_tick = 64;
	bool update_surfels_next_tick = false;
	float octree_total_extension = 512.0;

	void generate_surfels_via_compute_shader() const;
	void copy_update_positions_to_cpu();
	bool update_surfels_in_update_queue(int amount);
	void find_best_world_positions_to_update_lighting() const;
	void compute_shader_ao_approximation(uint32_t level, glm::uvec3 pos_in_octree) const;
	void sync_buffers() const;
	bool record_surfel_metadata = false;
	float sample_interval = 0.1f;
	double last_sample;

	void register_scene_data(Camera3D* camera, const framebuffer_object* surfel_framebuffer);


	//compute shaders
	compute_shader* insert_surfel_compute_shader;
	compute_shader* compute_shader_approximate_gi;
	compute_shader* compute_shader_find_least_shaded_pos;
	compute_shader* compute_shader_sync_buffers;

	void tick(double time_stamp);

	void swap_surfel_buffers() const;

	void record_state_time(double time, double delta, int state);

private:
	Scene* scene_;
	Camera3D* camera_ = nullptr; //camera object used to determine the center of view 


	//this array contains the indices of octree nodes that have been updated
	ssbo<glm::uint>* updated_octree_positions_;
	ssbo_double_buffer<surfel_gpu>* surfel_ssbo_consumer_;
	ssbo_double_buffer<surfel_octree_element>* surfels_octree_consumer_;
	ssbo<surfel_allocation_metadata>* surfel_allocation_metadata_;
	ssbo<glm::vec4>* least_shaded_regions_;

	GLsync compute_fence_state_machine_;

	//for debugging and statistics
	std::vector<std::pair<double, ::surfel_allocation_metadata>> meta_data_history_ = std::vector<std::pair<
		double, ::surfel_allocation_metadata>>();
	std::vector<state_history> state_pass_time_history_ = std::vector<state_history>();
	GLuint time_query_ = 0;
	double recording_start_ = 0.0;
	bool draw_debug_boxes_ = false;


	glm::uvec3 get_bucket_coordinates_from_ws(glm::vec3 ws_pos, int level) const;
	glm::vec3 get_ws_bucket_lowest_edge_from_octree_index(int layer, glm::uvec3 octree_index) const;
	static unsigned int get_bucket_amount_at_level(unsigned int level);
	float node_size_at_level(unsigned int level) const;

	surfel_parameters surfel_parameters_;
	

	std::vector<std::pair<unsigned, std::array<unsigned, 3>>> sample_positons_ = {};

	/*
	states:
	0 -> inserting surfels
	1 -> computing AO
	2 -> copying from compute buffer to backbuffer
	*/
	int manager_state_ = 0;
	int last_state_ = 0;
	bool last_updated_regions_available_ = false;

	
	void dump_metadata_history() const;
	void clear_surfels_on_gpu();

};
