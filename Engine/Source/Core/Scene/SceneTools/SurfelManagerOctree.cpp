#include "SurfelManagerOctree.h"

#include <numbers>
#include <random>
#include <utility>
#include <gtx/string_cast.hpp>

#include "../Camera3D.h"
#include "../RayCast.h"
#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../../../Util/math_util.h"
#include "../../Renderer/Shader/compute_shader.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"
#include "../../Renderer/Texture/texture_buffer_object.h"

class RayCast;

SurfelManagerOctree::SurfelManagerOctree(Scene* scene)
{
	scene_ = scene;
	surfel_ssbo_producer_ = new ssbo<surfel_gpu>();
	surfel_ssbo_producer_->init_buffer_object(SURFELS_BUCKET_AMOUNT * SURFEL_BUCKET_SIZE_ON_GPU, 0);

	surfel_ssbo_consumer_ = new ssbo_double_buffer<surfel_gpu>();
	surfel_ssbo_consumer_->init(SURFELS_BUCKET_AMOUNT * SURFEL_BUCKET_SIZE_ON_GPU, 3, 4);
	surfel_ssbo_consumer_->bind_back(3);
	surfel_ssbo_consumer_->bind_front(4);


	surfels_octree_producer_ = new ssbo<surfel_octree_element>();
	surfels_octree_producer_->init_buffer_object(SURFEL_OCTREE_SIZE, 1);

	surfels_octree_consumer_ = new ssbo_double_buffer<surfel_octree_element>();
	surfels_octree_consumer_->init(SURFEL_OCTREE_SIZE, 5, 6);
	surfels_octree_consumer_->bind_back(5);
	surfels_octree_consumer_->bind_front(6);

	surfel_allocation_metadata = new ssbo<::surfel_allocation_metadata>();
	surfel_allocation_metadata->init_buffer_object(1, 2);
	surfel_allocation_metadata->insert_data(new struct surfel_allocation_metadata(1, 1), 0);

	least_shaded_regions = new ssbo<glm::vec4>();
	least_shaded_regions->init_buffer_object(4096, 7);

	insert_surfel_compute_shader = new compute_shader();
	insert_surfel_compute_shader->loadFromFile("EngineContent/ComputeShader/SurfelInserter.glsl");
	insert_surfel_compute_shader->compileShader();

	compute_shader_approxmiate_ao = new compute_shader();
	compute_shader_approxmiate_ao->loadFromFile("EngineContent/ComputeShader/SurfelAoApproximator.glsl");
	compute_shader_approxmiate_ao->compileShader();

	compute_shader_find_least_shaded_pos = new compute_shader();
	compute_shader_find_least_shaded_pos->loadFromFile("EngineContent/ComputeShader/LowestPosFinder.glsl");
	compute_shader_find_least_shaded_pos->compileShader();
}

void SurfelManagerOctree::clear_samples()
{
	surfels_.clear();
}

void SurfelManagerOctree::draw_ui()
{

	if (ImGui::Button("Clear Surfels on GPU"))
	{
		clear_surfels_on_gpu_();
	}
	if (ImGui::Button("Generate surfels vis compute shader"))
	{
		generate_surfels_via_compute_shader();
	}

	ImGui::Checkbox("Update Surfels", &update_surfels_next_tick);
	ImGui::Checkbox("Draw boxes on update nodes", &draw_debug_boxes_);
	ImGui::DragInt("Surfel GI updates per tick", &surfel_gi_updates_per_tick);
}


int SurfelManagerOctree::get_octree_level_for_surfel(const surfel* surfel)
{
	auto d = surfel->radius * 2.0f;
	int level_surfel = ceil(log2(d));
	int level_bounds = ceil(log2(octree_total_extension));
	return std::min(level_bounds - level_surfel, octree_levels);
}

static uint32_t combine(const uint32_t a, const uint32_t b, const uint32_t mask)
{
	return (a & ~mask) | (b & mask);
}

static uint8_t get_pos_of_next_surfel_index_(glm::vec3 pos_relative)
{
	uint8_t r = 0b00000000;
	if (pos_relative.x >= 0)
	{
		r |= (1 << 2);
	}

	if (pos_relative.y >= 0)
	{
		r |= (1 << 1);
	}

	if (pos_relative.z >= 0)
	{
		r |= (1 << 0);
	}
	return r;
}

bool SurfelManagerOctree::insert_surfel_into_octree(surfel* surfel)
{
	auto target_level = get_octree_level_for_surfel(surfel);
	return insert_surfel_into_octree_recursive(surfel, 0, {0, 0, 0}, target_level, 0, true, surfel->mean);
}

void SurfelManagerOctree::generate_surfels_via_compute_shader() const
{
	if (camera_ != nullptr)
	{
		auto t = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0);
		if (t != nullptr)
		{

			std::random_device rd; // Seed the random number generator
			std::mt19937 gen(rd()); // Mersenne Twister PRNG
			std::uniform_real_distribution<float> float_dist_0_1(0.0f, 1.0f);
			
			insert_surfel_compute_shader->use();
			insert_surfel_compute_shader->set_uniform_vec3_f("camera_position",
			                                                 glm::value_ptr(camera_->getWorldPosition()));
			insert_surfel_compute_shader->set_uniform_vec3_f("random_offset", glm::value_ptr(glm::vec3(float_dist_0_1(gen), float_dist_0_1(gen),0.0 )));

			glDispatchCompute(t->width(), t->height(), 1);
			//glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
		}
	}
}

void SurfelManagerOctree::find_surfels_to_update()
{
	find_best_world_positions_to_update_lighting();
	
	GLsync computeFence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);

	GLenum result;
	do {
		result = glClientWaitSync(computeFence, GL_SYNC_FLUSH_COMMANDS_BIT, 1'000'000); // timeout in nanoseconds
	} while (result == GL_TIMEOUT_EXPIRED);

	glDeleteSync(computeFence);
	
	auto p = static_cast<glm::vec4*>(least_shaded_regions->write_ssbo_to_cpu());

	auto width = static_cast<int>(camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->width() / 32) + 1;
	auto height = static_cast<int>(camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->height() / 32) + 1;
	
	least_shaded_regions->unmap();

	std::set<std::pair<unsigned, std::array<unsigned, 3>>> sample_positons_set = {};

	for (int j = 0; j < width * height; j++)
	{
		
		glm::vec4 c = p[j];
		if (c.w < 0)
		{
			continue;
		}
		//camera_->get_camera()->get_render_target()->read_pixel(float_dist_0_1(gen),float_dist_0_1(gen),0,&c);

		int level = c.w;
		auto b = get_bucket_coordinates_from_ws(glm::vec3(c), level);
		auto g = std::pair<unsigned, std::array<unsigned, 3>>(static_cast<unsigned>(level),{b.x,b.y,b.z});
		sample_positons_set.insert(g);
	}
	sample_positons_ = std::vector(sample_positons_set.begin(), sample_positons_set.end()); 
}

bool SurfelManagerOctree::update_surfels_in_update_queue(int amount)
{
	auto queue_size = static_cast<int>(sample_positons_.size());
	amount = std::min(amount, queue_size);
	
	for (int i = 0; i < amount; i++)
	{
		auto sample_positon = sample_positons_.back();
		unsigned int level = sample_positon.first;
		glm::uvec3 coordinates = {sample_positon.second[0],sample_positon.second[1],sample_positon.second[2]};
		auto b = StructBoundingBox();
		b.min = get_ws_bucket_lowest_edge_from_octree_index(level, coordinates);
		b.max = b.min + node_size_at_level(level);
		if (draw_debug_boxes_) scene_->get_debug_tools()->draw_debug_cube(&b);
		compute_shader_ao_approximation(level, coordinates);

		sample_positons_.pop_back();
	}
	return sample_positons_.empty();
}

void SurfelManagerOctree::find_best_world_positions_to_update_lighting() const
{
	auto width = static_cast<int>(camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->width() / 32);
	auto height = static_cast<int>(camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->height() / 32);
	compute_shader_find_least_shaded_pos->use();
	glDispatchCompute(width, height, 1);
}

void SurfelManagerOctree::compute_shader_ao_approximation(uint32_t level, glm::uvec3 pos_in_octree) const
{
		compute_shader_approxmiate_ao->use();
		compute_shader_approxmiate_ao->setUniformInt("offset_id", 0);
		compute_shader_approxmiate_ao->setUniformInt("calculation_level", level);
		compute_shader_approxmiate_ao->set_uniform_vec3_u("pos_ws_start", value_ptr(pos_in_octree));
		glDispatchCompute(128, 1, 1);
	
}

bool SurfelManagerOctree::remove_surfel(const surfel* surfel)
{
	for (auto bucket_pointer : surfel->in_surfel_buckets)
	{
		//remove surfel references in buckets
		auto octree_element = &octree_[bucket_pointer];
		int index_in_bucket = get_surfel_pos_in_bucket(bucket_pointer, surfel);
		if (index_in_bucket < 0)
		{
			continue;
		}

		//remove reference in octree metadata
		octree_metadata_[bucket_pointer].surfels->erase(
			octree_metadata_[bucket_pointer].surfels->begin() + index_in_bucket);

		//UPDATE OCTREE
		auto surfel_amount_before_update = get_surfel_count_in_octree_element(octree_element);
		decrement_surfel_count_in_octree_element(octree_element); //decrement the amount of surfels at index in octree 
		update_octree_data_on_gpu(bucket_pointer); //upload updated data to gpu

		//UPDATE BUCKET 
		auto bucket_index = octree_element->surfels_at_layer_pointer;
		//update bucket on GPU
		remove_surfel_from_bucket_on_gpu(bucket_index, index_in_bucket, surfel_amount_before_update);
	}

	std::erase_if(surfels_,
	              [&](const std::unique_ptr<::surfel>& ptr)
	              {
		              return ptr.get() == surfel;
	              });

	return true;
}

void SurfelManagerOctree::get_overlapping_octree_elements(glm::vec3 center, float radius, int layer,
                                                          std::vector<glm::vec3>& result) const
{
	auto surfel_bucket_center = get_surfel_bucket_center(center, layer);

	float bucket_size = octree_total_extension / pow(2, layer);


	StructBoundingSphere b = {center, radius};
	StructBoundingBox bb = {};
	constexpr glm::vec3 component_multiplier[8] = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, -1.0f),
		glm::vec3(1.0f, -1.0f, 1.0f),
		glm::vec3(1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, 1.0f, 1.0f),
		glm::vec3(-1.0f, 1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, 1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
	};

	auto half_size = glm::vec3(0.5) * bucket_size;
	auto quarter_size = glm::vec3(0.5) * half_size;
	for (int i = 0; i < 8; i++)
	{
		auto center_neighbour = surfel_bucket_center + component_multiplier[i] * quarter_size;
		bb.min = center_neighbour - quarter_size;
		bb.max = center_neighbour + quarter_size;
		if (BoundingBoxHelper::are_overlapping(&bb, &b))
		{
			result.push_back(center_neighbour);
		}
	}
}

bool SurfelManagerOctree::insert_surfel_in_surrounding_buckets(surfel* surfel,
                                                               int target_layer)
{
	auto surfel_bucket_center = get_surfel_bucket_center(surfel->mean, target_layer);

	auto center_pos_v = surfel->mean - surfel_bucket_center;
	center_pos_v.x = center_pos_v.x > 0 ? 1.0f : -1.0f;
	center_pos_v.y = center_pos_v.y > 0 ? 1.0f : -1.0f;
	center_pos_v.z = center_pos_v.z > 0 ? 1.0f : -1.0f;
	center_pos_v = normalize(center_pos_v);

	float bucket_size = octree_total_extension / pow(2, target_layer);


	StructBoundingSphere b = {surfel->mean, surfel->radius};
	StructBoundingBox bb = {};
	constexpr glm::vec3 component_multiplier[8] = {
		glm::vec3(1.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 0.0f),
	};

	auto half_size = glm::vec3(0.5) * bucket_size;
	for (int i = 0; i < 7; i++)
	{
		auto center_neighbour = surfel_bucket_center + glm::normalize(center_pos_v * component_multiplier[i]) *
			bucket_size;
		bb.min = center_neighbour - half_size;
		bb.max = center_neighbour + half_size;
		if (BoundingBoxHelper::are_overlapping(&bb, &b))
		{
			insert_surfel_into_octree_recursive(surfel, 0, {0, 0, 0}, target_layer, 0, false, center_neighbour);
		}
	}

	return true;
}


glm::vec3 SurfelManagerOctree::get_surfel_bucket_center(glm::vec3 ws_pos, int level) const
{
	if (level == 0) return glm::vec3(0, 0, 0);
	float bucket_amount_at_level = pow(2, level);
	auto bucket_extension_ws = octree_total_extension / bucket_amount_at_level;
	ws_pos /= octree_total_extension * 0.5;
	ws_pos *= bucket_amount_at_level * 0.5f;
	ws_pos = glm::floor(ws_pos) + 0.5f;
	return ws_pos * bucket_extension_ws;
}

glm::uvec3 SurfelManagerOctree::get_bucket_coordinates_from_ws(glm::vec3 ws_pos, int level) const
{
	auto node_size = node_size_at_level(level);
	auto p = glm::uvec3(floor((ws_pos + octree_total_extension * 0.5f) / node_size));
	return glm::clamp(p, glm::uvec3(0), glm::uvec3(get_bucket_amount_at_level(level)));
}


glm::vec3 SurfelManagerOctree::get_center_of_sub_octree_level(int current_layer, glm::vec3 current_center,
                                                              glm::vec<3, float> pos_relative) const
{
	return current_center + glm::vec3(
		pos_relative.x >= 0 ? 1.0 : -1.0,
		pos_relative.y >= 0 ? 1.0 : -1.0,
		pos_relative.z >= 0 ? 1.0 : -1.0
	) * 0.5f * (octree_total_extension / powf(2.0f, 1 + current_layer));
}

glm::vec3 SurfelManagerOctree::get_ws_bucket_lowest_edge_from_octree_index(int layer, glm::uvec3 octree_index) const
{
	return glm::vec3(-octree_total_extension * 0.5f) + glm::vec3(octree_index) * node_size_at_level(layer);
}

glm::vec3 SurfelManagerOctree::get_ws_bucket_center_from_octree_index(int layer, glm::uvec3 octree_index) const
{
	return get_ws_bucket_lowest_edge_from_octree_index(layer, octree_index) + glm::vec3(node_size_at_level(layer)*0.5f);
}	

unsigned int SurfelManagerOctree::get_bucket_amount_at_level(unsigned int level)
{
	return 1u<<level;
}

float SurfelManagerOctree::node_size_at_level(unsigned int level) const
{
	return octree_total_extension / static_cast<float>(1 << level);
}

bool SurfelManagerOctree::insert_surfel_into_octree_recursive(surfel* surfel_to_insert, int current_layer,
                                                              glm::vec3 current_center, int target_layer,
                                                              int current_octree_element_index,
                                                              bool insert_into_surroundings, glm::vec3 target_pos)
{
	auto current_octree_element = &octree_[current_octree_element_index];
	auto current_octree_metadata_element = &octree_metadata_[current_octree_element_index];
	if (target_layer < current_layer)
	{
		return false;
	}
	if (target_layer == current_layer)
	{
		//insert -> write this to gpu memory
		uint32_t surfel_bucket_pointer;
		auto surfel_count = get_surfel_count_in_octree_element(current_octree_element);
		if (surfel_count <= 0)
		{
			//create new
			create_space_for_new_surfel_data(surfel_bucket_pointer, 1);
			current_octree_element->surfels_at_layer_pointer = surfel_bucket_pointer;
		}
		else if (surfel_count >= SURFEL_BUCKET_SIZE_ON_GPU)
		{
			//TODO resize surfel buckets when more space is needed  
			memory_limitation_bucket_size++;
			return false;
		}


		//associate surfel with bucket and bucket with surfel 

		if (insert_into_surroundings)
		{
			insert_surfel_in_surrounding_buckets(surfel_to_insert, target_layer);
		}
		auto insertion_index = current_octree_element->surfels_at_layer_pointer + surfel_count;

		if (upload_surfel_data(surfel_to_insert, insertion_index))
		{
			current_octree_metadata_element->surfels->push_back(surfel_to_insert);
			surfel_to_insert->in_surfel_buckets.push_back(current_octree_element_index);

			increment_surfel_count_in_octree_element(current_octree_element);
			//update octree
			update_octree_data_on_gpu(current_octree_element_index); //upload updated data to gpu

			return true; //data is successfully uploaded to gpu memeory 
		}
		return false; //data couldn't be uploaded
	}

	auto pos_relative = target_pos - current_center;
	auto next_index = get_pos_of_next_surfel_index_(pos_relative);
	uint32_t next_octree_index;
	if (is_child_octree_bit_set_at_(current_octree_element, next_index))
	{
		//child already exists
		next_octree_index = current_octree_element->next_layer_surfels_pointer[next_index];
	}
	else
	{
		uint32_t new_index;
		if (create_new_octree_element(new_index, current_octree_element_index))
		{
			//could create new index
			next_octree_index = new_index;

			//set the bit to true at the index and insert the next pointer at the corresponding position
			current_octree_element->next_layer_surfels_pointer[next_index] = next_octree_index;
			set_child_octree_bit_at_(current_octree_element, next_index);
			update_octree_data_on_gpu(current_octree_element_index); //upload updated data to gpu
		}
		else
		{
			//alrady full
			return false;
		}
	}
	glm::vec3 next_center = get_center_of_sub_octree_level(current_layer, current_center, pos_relative);


	return insert_surfel_into_octree_recursive(surfel_to_insert, ++current_layer, next_center, target_layer,
	                                           next_octree_index, insert_into_surroundings, target_pos);
}


void SurfelManagerOctree::get_surfels_in_radius_recursive(glm::vec3 ws_pos, float radius, int current_layer,
                                                          glm::vec3 current_center, int current_octree_element_index,
                                                          std::set<surfel*>* found_surfels)
{
	auto o = &octree_[current_octree_element_index];
	auto o_meta = &octree_metadata_[current_octree_element_index];

	//check surfels at layer
	auto surfel_amount = get_surfel_count_in_octree_element(o);
	for (int i = 0; i < surfel_amount; i++)
	{
		auto s = o_meta->surfels->at(i);
		if (radius + 0.01f > s->radius + glm::distance(s->mean, ws_pos))
		{
			//surfel is inside radius
			found_surfels->insert(s);
		}
	}

	//search in next buckets
	auto pos_relative = ws_pos - current_center;
	glm::vec3 next_center = get_center_of_sub_octree_level(current_layer, current_center, pos_relative);

	std::vector<glm::vec3> centers_to_check = std::vector<glm::vec3>();
	get_overlapping_octree_elements(ws_pos, radius, current_layer, centers_to_check);

	for (auto to_check : centers_to_check)
	{
		auto next_index = get_pos_of_next_surfel_index_(to_check - current_center);
		if (is_child_octree_bit_set_at_(o, next_index))
		{
			auto next_surfel = o->next_layer_surfels_pointer[next_index];
			get_surfels_in_radius_recursive(ws_pos, radius, current_layer + 1, to_check, next_surfel, found_surfels);
		}
	}
}


bool SurfelManagerOctree::create_new_octree_element(uint32_t& index, uint32_t parent_index)
{
	if (next_free_spot_in_octree_ >= SURFEL_OCTREE_SIZE)
	{
		memory_limitation_octree_size++;
		return false;
	}

	octree_[next_free_spot_in_octree_] = {};

	octree_metadata_[next_free_spot_in_octree_] = {
		.parent = static_cast<int>(parent_index)
	};

	next_free_spot_in_octree_++;
	index = next_free_spot_in_octree_ - 1;
	return true;
}

bool SurfelManagerOctree::create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array, uint32_t allocation_size)
{
	if (SURFELS_BUCKET_AMOUNT <= surfel_stack_pointer / SURFEL_BUCKET_SIZE_ON_GPU)
	{
		//the surfel buffer on the gpu is already full
		memory_limitation_count_bottom_size++;
		return false;
	}

	//find free spot


	pointer_ins_surfel_array = surfel_stack_pointer;
	surfel_stack_pointer += SURFEL_BUCKET_SIZE_ON_GPU;
	return true;
}

//TODO: veryyyy inefficient
bool SurfelManagerOctree::upload_surfel_data(surfel* surfel_to_insert, unsigned int insertion_index) const
{
	const auto temp_surfel = surfel_gpu(
		{surfel_to_insert->mean, surfel_to_insert->radius}, {surfel_to_insert->normal, 0.0f},
		{surfel_to_insert->diffuse_irradiance, 1.0f});
	return surfel_ssbo_producer_->insert_data(&temp_surfel, insertion_index);
}

bool SurfelManagerOctree::update_surfel_data(surfel* surfel_to_insert) const
{
	//search through all the buckets this surfel is contained in and find its location in each

	bool s = true;

	for (auto bucket : surfel_to_insert->in_surfel_buckets)
	{
		auto octree_meta = &octree_metadata_[bucket];
		auto octree = &octree_[bucket];
		auto index = get_surfel_pos_in_bucket(bucket, surfel_to_insert);
		if (index < 0)
		{
			s = false;
		}
		else
		{
			s &= upload_surfel_data(surfel_to_insert, octree->surfels_at_layer_pointer + index);
		}
	}

	return s;
}

int SurfelManagerOctree::get_surfel_pos_in_bucket(unsigned int bucket_index, const surfel* surfel_to_find) const
{
	auto octree_meta = &octree_metadata_[bucket_index];
	for (int i = 0; i < octree_meta->surfels->size(); i++)
	{
		if (octree_meta->surfels->at(i) == surfel_to_find)
		{
			return i;
		}
	}
	scene_->get_global_context()->logger->print_error(std::format(
		"surfel at: {} should be in bucket {} but wasn't found", glm::to_string(surfel_to_find->mean), bucket_index));
	return -1;
}

void SurfelManagerOctree::dump_surfel_octree_to_gpu_memory()
{
	surfels_octree_producer_->insert_data(&octree_[0], 0, next_free_spot_in_octree_);
}

void SurfelManagerOctree::update_octree_data_on_gpu(unsigned int octree_index)
{
	surfels_octree_producer_->insert_data(&octree_[octree_index], octree_index, 1);
}

void SurfelManagerOctree::remove_surfel_from_bucket_on_gpu(unsigned int bucket_start, unsigned int index,
                                                           unsigned int last_bucket_element)
{
	if (index >= last_bucket_element)
	{
		return;
	}
	auto from = bucket_start + index + 1;
	auto to = bucket_start + index;
	auto length = last_bucket_element - index - 1;
	// ReSharper disable once CppExpressionWithoutSideEffects
	surfel_ssbo_producer_->move_data(from, to, length);
}

void SurfelManagerOctree::clear_surfels_on_gpu_() const
{
	surfel_allocation_metadata->insert_data(new struct surfel_allocation_metadata(1, 1), 0);
	surfel_ssbo_producer_->clear_data();
	surfels_octree_producer_->clear_data();
}


void SurfelManagerOctree::generate_surfels()
{
	clear_samples();
	//StructBoundingBox boundingbox = {lower_left, upper_right};
	//auto colliders_in_bb = std::vector<collider_modifier*>();
	auto colliders = scene_->get_colliders(VISIBILITY);

	auto points = std::vector<vertex>();
	for (collider_modifier* collider : colliders)
	{
		collider->scatter_points_on_surface(&points, points_per_square_meter);
	}

	std::random_device rd; // Seed the random number generator
	std::mt19937 gen(rd()); // Mersenne Twister PRNG
	std::uniform_real_distribution<float> float_dist_0_1(0.0f, 1.0f);


	for (auto point : points)
	{
		float radius = starting_radius;
		if (camera_ != nullptr)
			//{
			//	auto distance_camera_p = glm::distance(camera_->getWorldPosition(), point.position) +0.001f;
			//	auto distance_camera_p_sqrt=sqrt(distance_camera_p);
			//	auto r = float_dist_0_1(gen);
			//	if (r > (1/distance_camera_p))
			//	{
			//		continue;
			//	}
			//
			//	radius = std::max(distance_camera_p * starting_radius, starting_radius);
			//}

			surfels_.push_back(std::make_unique<surfel>(surfel
				{
					.mean = point.position,
					.normal = point.normal,
					.diffuse_irradiance = {1, 1, 1},
					.radius = radius
				}));
	}
}


void SurfelManagerOctree::init_surfels_buffer()
{
	if (has_surfels_buffer_)
	{
		scene_->get_global_context()->logger->print_warning("Scene already initialized surfel buffers");
		return;
	}

	scene_->get_global_context()->logger->print_info("Initializing surfel buffer...");
	scene_->get_global_context()->logger->print_info(std::format("Surfel buffer size : {}", SURFEL_OCTREE_SIZE));


	scene_->get_global_context()->logger->print_info(std::format("Surfel buffer total memory : {}",
	                                                             sizeof(glm::vec3) * SURFEL_OCTREE_SIZE * 3 + sizeof(
		                                                             float) * SURFEL_OCTREE_SIZE)
	);
	scene_->get_global_context()->logger->print_info(std::format("Surfel grid total memory : {}",
	                                                             sizeof(unsigned int) * 2 * SURFELS_BUCKET_AMOUNT *
	                                                             SURFELS_BUCKET_AMOUNT * SURFELS_BUCKET_AMOUNT)
	);

	has_surfels_buffer_ = true;
}

void SurfelManagerOctree::reset_surfels_buffer()
{
	next_free_spot_in_octree_ = 1;
	last_updated_surfel = 0;
	surfel_stack_pointer = 0;
	delete[] octree_;
	octree_ = new surfel_octree_element[SURFEL_OCTREE_SIZE];
	octree_[0] = {.surfels_at_layer_amount = 0, .surfels_at_layer_pointer = 0, .next_layer_surfels_pointer = {}};

	delete[] octree_metadata_;
	octree_metadata_ = new surfel_octree_metadata[SURFEL_OCTREE_SIZE];
	octree_metadata_[0] = {.parent = -1};
}

void SurfelManagerOctree::recalculate_surfels()
{
	if (!has_surfels_buffer_)
	{
		init_surfels_buffer();
	}
	reset_surfels_buffer();


	memory_limitation_octree_size = 0;
	memory_limitation_count_bottom_size = 0;
	memory_limitation_bucket_size = 0;

	int sucessfull_inserts = 0;
	for (auto& s : surfels_)
	{
		sucessfull_inserts += insert_surfel_into_octree(s.get()) ? 1 : 0;
	}

	//dump_surfel_octree_to_gpu_memory();
	scene_->get_global_context()->logger->print_info(std::format(
		"surfel octree created {} surfels out of {} have been inserted ",
		sucessfull_inserts, surfels_.size()));

	scene_->get_global_context()->logger->print_info(
		std::format("Memory limitation for octree size was met in {} cases", memory_limitation_octree_size));
	scene_->get_global_context()->logger->print_info(std::format(
		"Memory limitation for bottom level buckets was met in {} cases", memory_limitation_count_bottom_size));
	scene_->get_global_context()->logger->print_info(
		std::format("Memory limitation for single bucket size was met in {} cases", memory_limitation_bucket_size));
}

void SurfelManagerOctree::create_packed_circles(glm::vec3 center, glm::vec3 normal, float radius, glm::vec3 color)
{
	//center
	auto tangent_bi_tangent = math_util::get_tangent_and_bi_tangent(normal);

	glm::vec3 centers[7] = {};
	centers[0] = center;
	auto radius_new = radius / 3.0f;
	for (int i = 1; i < 7; i++)
	{
		auto angle = ((glm::pi<float>() * 2.0f) / 6.0f) * i;
		centers[i] = center + (tangent_bi_tangent.first * sin(angle) + tangent_bi_tangent.second * cos(angle)) *
			radius_new * 2.0f;
	}

	for (int i = 0; i < 7; i++)
	{
		auto new_surfel = surfel();
		new_surfel.radius = radius_new + radius_new * (std::numbers::sqrt2 - 1); //TODO: prove this 
		new_surfel.mean = centers[i];
		new_surfel.normal = normal;
		new_surfel.samples = gi_primary_rays;
		new_surfel.diffuse_irradiance = color;
		new_surfel.diffuse_irradiance_samples[0] = color;
		new_surfel.diffuse_irradiance_samples[1] = color;
		new_surfel.diffuse_irradiance_samples[2] = color;
		new_surfel.diffuse_irradiance_samples[3] = color;
		insert_surfel(new_surfel);
	}
}

void SurfelManagerOctree::update_surfels()
{
	if (surfels_.empty() || !update_surfels_next_tick) return;
	for (int i = 0; i < update_surfels_per_tick; i++)
	{
		if (last_updated_surfel >= surfels_.size())
		{
			last_updated_surfel = 0;
		}
		surfel* s = surfels_.at(last_updated_surfel).get();

		auto tangent_bi_tangent = math_util::get_tangent_and_bi_tangent(s->normal);
		glm::vec3 irradiance_center_sum = glm::vec3(0);
		int wall_hits = 0;
		for (int sample_index = 0; sample_index < 4; sample_index++)
		{
			auto offset_2d = surfel_sample_offsets[sample_index];
			auto offset_3d = tangent_bi_tangent.first * offset_2d.x + tangent_bi_tangent.second * offset_2d.y;
			offset_3d *= s->radius;

			auto irradiance_info = scene_->get_irradiance_information(s->mean + offset_3d, s->normal, gi_primary_rays);

			if (irradiance_info.inside_wall)
			{
				wall_hits++;
				s->diffuse_irradiance_samples[sample_index] = {0, 0, 0};
			}
			else
			{
				s->diffuse_irradiance_samples[sample_index] = s->diffuse_irradiance_samples[sample_index]
					* static_cast<float>(s->samples) + irradiance_info.color * static_cast<float>(gi_primary_rays);
				s->diffuse_irradiance_samples[sample_index] /= static_cast<float>(s->samples + gi_primary_rays);
				irradiance_center_sum += s->diffuse_irradiance_samples[sample_index];
			}
		}

		s->samples += gi_primary_rays;
		s->diffuse_irradiance = (wall_hits >= 4)
			                        ? glm::vec3(0.0)
			                        : irradiance_center_sum / static_cast<float>(4 - wall_hits);
		bool update = true;

		auto illumination_d = get_surfel_illumination_gradient(s);

		if (glm::length(illumination_d) > illumination_derivative_threshold && s->radius >= minimal_surfel_radius)
		{
			//internal illumination gradient is larger then threshold
			create_packed_circles(s->mean, s->normal, s->radius, s->diffuse_irradiance);
			remove_surfel(s);
			update = false;
		}
		//check if surfel has a neighbour with similar illumination
		//check if surfel can be merged with neighbour

		auto neighbour_surfel = get_closest_neighbour_on_level(s);
		if (neighbour_surfel != nullptr && update)
		{
			auto gradient = abs(length(get_illumination_gradient(s, neighbour_surfel)));
			auto gradient_adjusted = gradient * (s->radius + neighbour_surfel->radius);
			if (gradient_adjusted < 0.2 && glm::distance(neighbour_surfel->mean, s->mean) < neighbour_surfel->radius + s
				->
				radius)
			{
				auto combined_surfel = get_combining_surfel(s, neighbour_surfel);
				auto overlaps = std::set<surfel*>();
				get_surfels_in_radius_recursive(combined_surfel.mean, combined_surfel.radius, 0, {0, 0, 0}, 0,
				                                &overlaps);

				if (merge_surfels(s, neighbour_surfel, combined_surfel, overlaps, 0.2))
				{
					//surfels were merged
					scene_->get_debug_tools()->draw_debug_point(combined_surfel.mean, 0.2, {1.0, 0.0, 0.0},
					                                            combined_surfel.radius);
					update = false;
				}
			}
		}


		if (update)
		{
			update_surfel_data(s);
		}
		last_updated_surfel++;
		if (last_updated_surfel >= surfels_.size())
		{
			last_updated_surfel = 0;
		}
	}
}

surfel* SurfelManagerOctree::get_closest_neighbour_on_level(const surfel* s) const
{
	constexpr float normal_difference = 0.9f;
	float smallest_distance = std::numeric_limits<float>::max();
	surfel* closest_surfel = nullptr;
	for (auto bucket_pointer : s->in_surfel_buckets)
	{
		for (auto neighbouring_surfel : *octree_metadata_[bucket_pointer].surfels)
		{
			if (s != neighbouring_surfel && glm::dot(s->normal, neighbouring_surfel->normal) > normal_difference)
			{
				auto d = glm::distance(s->mean, neighbouring_surfel->mean);
				if (d < smallest_distance)
				{
					smallest_distance = d;
					closest_surfel = neighbouring_surfel;
				}
			}
		}
	}
	return closest_surfel;
}

glm::vec3 SurfelManagerOctree::get_illumination_gradient(const surfel* s_1, const surfel* s_2)
{
	auto d = glm::distance(s_1->mean, s_2->mean);
	return (s_2->diffuse_irradiance - s_1->diffuse_irradiance) / d;
}

glm::vec2 SurfelManagerOctree::get_surfel_illumination_gradient(surfel* s)
{
	//central difference x:
	auto d_x =
		math_util::component_average(s->diffuse_irradiance_samples[0] - s->diffuse_irradiance_samples[1]); // /
	//(s->radius * 2.0f);
	auto d_y =
		math_util::component_average(s->diffuse_irradiance_samples[2] - s->diffuse_irradiance_samples[3]); // /
	//(s->radius * 2.0f);
	return {d_x, d_y};
}


surfel SurfelManagerOctree::get_combining_surfel(const surfel* s_1, const surfel* s_2)
{
	auto new_normal = (s_1->normal + s_2->normal) * 0.5f;
	auto new_irradiance = math_util::get_weighted_average(s_1->diffuse_irradiance, s_2->diffuse_irradiance,
	                                                      s_1->samples, s_2->samples);

	auto distance = glm::distance(s_1->mean, s_2->mean);
	auto new_radius = 0.0f;
	glm::vec3 new_center;

	if (distance + std::min(s_1->radius, s_2->radius) <= std::max(s_1->radius, s_2->radius))
	{
		//one circle completely encloses the other 
		if (s_1->radius > s_2->radius)
		{
			new_center = s_1->mean;
			new_radius = s_1->radius;
		}
		else
		{
			new_center = s_2->mean;
			new_radius = s_2->radius;
		}
	}
	else
	{
		new_radius = 0.5f * (distance + s_1->radius + s_2->radius);
		auto interpolation_factor = (new_radius - s_1->radius) / distance;
		new_center = s_1->mean + interpolation_factor * (s_2->mean - s_1->mean);
	}


	auto new_samples = s_1->samples + s_2->samples;

	auto new_surfel = surfel{
		.mean = new_center, // new center 
		.normal = new_normal, // new normal
		.diffuse_irradiance = new_irradiance,
		.radius = new_radius,
		.in_surfel_buckets = {},
		.samples = new_samples,
		.diffuse_irradiance_samples = {
			math_util::get_weighted_average(s_1->diffuse_irradiance_samples[0], s_2->diffuse_irradiance_samples[0],
			                                s_1->samples, s_2->samples),
			math_util::get_weighted_average(s_1->diffuse_irradiance_samples[1], s_2->diffuse_irradiance_samples[1],
			                                s_1->samples, s_2->samples),
			math_util::get_weighted_average(s_1->diffuse_irradiance_samples[2], s_2->diffuse_irradiance_samples[2],
			                                s_1->samples, s_2->samples),
			math_util::get_weighted_average(s_1->diffuse_irradiance_samples[3], s_2->diffuse_irradiance_samples[3],
			                                s_1->samples, s_2->samples)
		}
	};

	return new_surfel;
}

bool SurfelManagerOctree::merge_surfels(const surfel* s_1, const surfel* s_2, const surfel& new_surfel,
                                        std::set<surfel*>& additional_overlaps, float
                                        max_gradient_difference)
{
	if (additional_overlaps.size() > 2)
	{
		//there are more overlaps to check
		for (auto surfel_overlap : additional_overlaps)
		{
			if (surfel_overlap != s_1 && surfel_overlap != s_2)
			{
				if (abs(glm::length(get_illumination_gradient(s_1, surfel_overlap))) > max_gradient_difference ||
					abs(glm::length(get_illumination_gradient(s_2, surfel_overlap))) > max_gradient_difference)
				{
					return false;
				}
			}
		}
	}


	remove_surfel(s_1);
	remove_surfel(s_2);

	if (additional_overlaps.size() > 2)
	{
		//there are more overlaps to check
		for (auto surfel_overlap : additional_overlaps)
		{
			if (surfel_overlap != s_1 && surfel_overlap != s_2)
			{
				remove_surfel(surfel_overlap);
			}
		}
	}
	return insert_surfel(new_surfel);
}

bool SurfelManagerOctree::insert_surfel(const surfel& surfel_to_insert)
{
	auto new_surfel_pointer = std::make_unique<surfel>(surfel_to_insert);

	insert_surfel_into_octree(new_surfel_pointer.get());
	surfels_.push_back(std::move(new_surfel_pointer));
	return true;
}

void SurfelManagerOctree::register_scene_data(Camera3D* camera, const framebuffer_object* surfel_framebuffer)
{
	camera_ = camera;
	auto t_pos = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0);
	auto t_normal = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(1);
	auto t_albedo = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(2);
	auto t_emissive = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(5);

	auto surfel_framebuffer_texture = surfel_framebuffer->get_color_attachment_at_index(0);
	auto surfel_framebuffer_metadata_0_texture = surfel_framebuffer->get_color_attachment_at_index(1);
	auto surfel_framebuffer_metadata_1_texture = surfel_framebuffer->get_color_attachment_at_index(2);
	
	insert_surfel_compute_shader->addTexture(t_pos, "gPos");
	insert_surfel_compute_shader->addTexture(t_normal, "gNormal");
	insert_surfel_compute_shader->addTexture(t_albedo, "gAlbedo");
	insert_surfel_compute_shader->addTexture(t_emissive, "gEmissive");
	insert_surfel_compute_shader->addTexture(surfel_framebuffer_texture, "gSurfels");
	insert_surfel_compute_shader->addTexture(camera->get_scene()->get_scene_direct_light()->light_map(), "direct_light_map_texture");

	
	compute_shader_find_least_shaded_pos->addTexture(t_pos, "gPos");
	compute_shader_find_least_shaded_pos->addTexture(surfel_framebuffer_metadata_0_texture, "surfel_framebuffer_metadata_0");
	compute_shader_find_least_shaded_pos->addTexture(surfel_framebuffer_metadata_1_texture, "surfel_framebuffer_metadata_1");
	
}

void SurfelManagerOctree::tick()
{
	if (!update_surfels_next_tick)
	{
		return;
	}


	tick_amount_++;
	
	if (compute_fence_)
	{
		GLenum result = glClientWaitSync(compute_fence_, 0, 0); // non-blocking poll

		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
		{
			// Compute finished — safe to dispatch the next one
			glDeleteSync(compute_fence_);
			compute_fence_ = nullptr;
		}
		else
		{
			//std::printf(" |");
			return;
		}
	}
	switch (manager_state_)
	{
	case 0: //insert surfels
		swap_surfel_buffers();//swap front and backbuffer
		generate_surfels_via_compute_shader();
		//std::printf("\n run generate_surfels_via_compute_shader");
		compute_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 1;
		break;
	case 1: //compute ao
		find_surfels_to_update();
		//std::printf("\n run update_surfel_ao_via_compute_shader");
		compute_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 2;
	case 2:  // FALLTHROUGH_
		if (update_surfels_in_update_queue(surfel_gi_updates_per_tick))
		{
			manager_state_ = 3;
		}
		compute_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		break;
	case 3: //copying from compute buffer to backbuffer
		copy_data_from_compute_to_back_buffer();
		//std::printf("\n run copy_data_from_compute_to_back_buffer");

		compute_fence_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 0;
		break;
	default:
		manager_state_ = 0;
	}
}

void SurfelManagerOctree::copy_data_from_compute_to_back_buffer() const
{
	surfel_ssbo_producer_->copy_buffer_data(surfel_ssbo_consumer_->back());
	surfels_octree_producer_->copy_buffer_data(surfels_octree_consumer_->back());
}

void SurfelManagerOctree::swap_surfel_buffers() const
{
	surfel_ssbo_consumer_->swap();
	surfels_octree_consumer_->swap();
}

int SurfelManagerOctree::get_manager_state() const
{
	return manager_state_;
}

bool SurfelManagerOctree::is_child_octree_bit_set_at_(const surfel_octree_element* surfel_octree_element,
                                                      const uint8_t pos)
{
	return (surfel_octree_element->surfels_at_layer_amount & (1 << (31 - pos))) != 0;
}

void SurfelManagerOctree::set_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, const uint8_t pos)
{
	surfel_octree_element->surfels_at_layer_amount |= (1 << (31 - pos));
}

void SurfelManagerOctree::unset_child_octree_bit_at_(surfel_octree_element* surfel_octree_element, uint8_t pos)
{
	surfel_octree_element->surfels_at_layer_amount &= ~(1 << (31 - pos));
}

bool SurfelManagerOctree::are_all_child_octree_bits_empty_(surfel_octree_element* surfel_octree_element)
{
	constexpr uint32_t mask = 0xFF000000;
	return (mask & surfel_octree_element->surfels_at_layer_amount) == 0;
}

void SurfelManagerOctree::increment_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element)
{
	constexpr uint32_t mask = 0xFF000000;
	uint32_t amount = get_surfel_count_in_octree_element(surfel_octree_element);
	amount++;
	surfel_octree_element->surfels_at_layer_amount = (surfel_octree_element->surfels_at_layer_amount & mask) | amount;
}

void SurfelManagerOctree::decrement_surfel_count_in_octree_element(surfel_octree_element* surfel_octree_element)
{
	constexpr uint32_t mask = 0xFF000000;
	uint32_t amount = get_surfel_count_in_octree_element(surfel_octree_element);
	amount--;
	surfel_octree_element->surfels_at_layer_amount = (surfel_octree_element->surfels_at_layer_amount & mask) | amount;
}

unsigned int SurfelManagerOctree::get_surfel_count_in_octree_element(const surfel_octree_element* surfel_octree_element)
{
	constexpr uint32_t mask = 0x00FFFFFF;
	return surfel_octree_element->surfels_at_layer_amount & mask;
}
