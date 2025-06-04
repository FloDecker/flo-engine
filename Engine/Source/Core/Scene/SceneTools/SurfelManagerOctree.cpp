#include "SurfelManagerOctree.h"

#include <random>
#include <utility>
#include <gtx/string_cast.hpp>

#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"
#include "../../Renderer/Texture/texture_buffer_object.h"

SurfelManagerOctree::SurfelManagerOctree(Scene* scene)
{
	scene_ = scene;
	surfels_texture_buffer_positions_ = new texture_buffer_object();
	surfels_texture_buffer_normals_ = new texture_buffer_object();
	surfels_texture_buffer_color_ = new texture_buffer_object();
	surfels_texture_buffer_radii_ = new texture_buffer_object();
	surfels_octree = new texture_buffer_object();
}

void SurfelManagerOctree::clear_samples()
{
	surfels_.clear();
}

void SurfelManagerOctree::draw_ui()
{
	if (ImGui::Button("Generate surfels"))
	{
		snap_samples_to_closest_surface();
		recalculate_surfels();
	}
	
	ImGui::Checkbox("Draw debug tools", &draw_debug_tools_);

	if (draw_debug_tools_)
	{
		for (const auto& sample : surfels_)
		{
			scene_->get_debug_tools()->draw_debug_point(sample.get()->mean);
		}
	}

	ImGui::DragFloat("points per square meter", &points_per_square_meter);
	ImGui::DragInt("surfel primary rays", &gi_primary_rays);
	ImGui::DragInt("surfel updates per tick", &update_surfels_per_tick);

	if (ImGui::Button("Delete first surfle"))
	{
		for (int i = 0; i < 100; i++)
		{
			remove_surfel(surfels_.at(0).get());
			surfels_.erase(surfels_.begin());
		}
	}
}


int SurfelManagerOctree::get_octree_level_for_surfel(const surfel* surfel)
{
	auto r = surfel->radius;
	int level_surfel = ceil(log2(r));
	int level_bounds = ceil(log2(total_extension));
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

	return false;
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

	float bucket_size = total_extension / pow(2, target_layer);


	StructBoundingSphere b = {surfel->mean, bucket_size * 0.5f};
	StructBoundingBox bb = {};
	constexpr glm::vec3 component_multiplier[8] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
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
	auto bucket_extension_ws = total_extension / bucket_amount_at_level;
	ws_pos /= total_extension * 0.5;
	ws_pos *= bucket_amount_at_level * 0.5f;
	ws_pos = glm::floor(ws_pos) + 0.5f;
	return ws_pos * bucket_extension_ws;
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
			create_space_for_new_surfel_data(surfel_bucket_pointer);
			current_octree_element->surfels_at_layer_pointer = surfel_bucket_pointer;
		}
		else if (surfel_count >= SURFEL_BUCKET_SIZE_ON_GPU)
		{
			//TODO resize surfel buckets when more space is needed  
			memory_limitation_bucket_size++;
			return false;
		}

		//TODO: this shouldnt happen in the first place, surfel is tried to be inserted in bucket that already contains it 
		//for (int i = 0; i < surfel_to_insert->octree_element_holding_surfel_surfel_index_in_bucket->size(); i++)
		//{
		//    if (std::cmp_equal(surfel_to_insert->octree_element_holding_surfel_surfel_index_in_bucket->at(i).first,
		//                       current_octree_element_index))
		//    {
		//        return false;
		//    }
		//}

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
		}
		else
		{
			//alrady full
			return false;
		}
	}
	glm::vec3 next_center = current_center + glm::vec3(
		pos_relative.x >= 0 ? 1.0 : -1.0,
		pos_relative.y >= 0 ? 1.0 : -1.0,
		pos_relative.z >= 0 ? 1.0 : -1.0
	) * 0.5f * (total_extension / powf(2.0f, current_layer + 1));


	return insert_surfel_into_octree_recursive(surfel_to_insert, ++current_layer, next_center, target_layer,
	                                           next_octree_index, insert_into_surroundings, target_pos);
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

bool SurfelManagerOctree::create_space_for_new_surfel_data(uint32_t& pointer_ins_surfel_array)
{
	if (SURFELS_BOTTOM_LEVEL_SIZE <= surfel_stack_pointer / SURFEL_BUCKET_SIZE_ON_GPU)
	{
		//the surfel buffer on the gpu is already full
		memory_limitation_count_bottom_size++;
		return false;
	}

	pointer_ins_surfel_array = surfel_stack_pointer;
	surfel_stack_pointer += SURFEL_BUCKET_SIZE_ON_GPU;
	return true;
}

//TODO: veryyyy inefficient
bool SurfelManagerOctree::upload_surfel_data(surfel* surfel_to_insert, unsigned int insertion_index) const
{
	bool s = true;
	s &= surfels_texture_buffer_positions_->update_vec3_single(&surfel_to_insert->mean, insertion_index);
	s &= surfels_texture_buffer_normals_->update_vec3_single(&surfel_to_insert->normal, insertion_index);
	s &= surfels_texture_buffer_color_->update_vec3_single(&surfel_to_insert->diffuse_irradiance, insertion_index);
	s &= surfels_texture_buffer_radii_->update_float_single(&surfel_to_insert->radius, insertion_index);
	return s;
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
	surfels_octree->update_u_int(&octree_[0].surfels_at_layer_amount, next_free_spot_in_octree_ * 10, 0);
}

void SurfelManagerOctree::update_octree_data_on_gpu(unsigned int octree_index)
{
	surfels_octree->update_u_int(&octree_[octree_index].surfels_at_layer_amount, 1, octree_index * 10);
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
	surfels_texture_buffer_positions_->move_data(sizeof(glm::vec3) * from, sizeof(glm::vec3) * to,
	                                             sizeof(glm::vec3) * length);
	surfels_texture_buffer_normals_->move_data(sizeof(glm::vec3) * from, sizeof(glm::vec3) * to,
	                                           sizeof(glm::vec3) * length);
	surfels_texture_buffer_color_->move_data(sizeof(glm::vec3) * from, sizeof(glm::vec3) * to,
	                                         sizeof(glm::vec3) * length);
	surfels_texture_buffer_radii_->move_data(sizeof(uint32_t) * from, sizeof(uint32_t) * to, sizeof(uint32_t) * length);
}

void SurfelManagerOctree::snap_samples_to_closest_surface()
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

	for (auto point : points)
	{
		//TODO :remove me thats a test
		//std::random_device rd; // Seed the random number generator
		//std::mt19937 gen(rd()); // Mersenne Twister PRNG
		//std::uniform_real_distribution<float> float_dist_0_1(1.0f / points_per_square_meter, 5.0f); 
		surfels_.push_back(std::make_unique<surfel>(surfel{
			point.position,
			point.normal,
			{1, 1, 1},
			1.0f / points_per_square_meter
		}));
	}
}


void SurfelManagerOctree::add_surfel_uniforms_to_shader(ShaderProgram* shader) const
{
	shader->addTexture(surfels_texture_buffer_color_, "surfels_texture_buffer_color_");
	shader->addTexture(surfels_texture_buffer_normals_, "surfels_texture_buffer_normals_");
	shader->addTexture(surfels_texture_buffer_positions_, "surfels_texture_buffer_positions_");
	shader->addTexture(surfels_texture_buffer_radii_, "surfels_texture_buffer_radii_");
	shader->addTexture(surfels_octree, "surfels_uniform_grid");
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

	surfels_octree->init_u_int(SURFEL_OCTREE_SIZE * 10);


	surfels_texture_buffer_positions_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
	surfels_texture_buffer_normals_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
	surfels_texture_buffer_color_->init_vec3(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);
	surfels_texture_buffer_radii_->init_float(SURFELS_BOTTOM_LEVEL_SIZE * SURFEL_BUCKET_SIZE_ON_GPU);

	scene_->get_global_context()->logger->print_info(std::format("Surfel buffer total memory : {}",
	                                                             sizeof(glm::vec3) * SURFEL_OCTREE_SIZE * 3 + sizeof(
		                                                             float) * SURFEL_OCTREE_SIZE)
	);
	scene_->get_global_context()->logger->print_info(std::format("Surfel grid total memory : {}",
	                                                             sizeof(unsigned int) * 2 * SURFELS_BOTTOM_LEVEL_SIZE *
	                                                             SURFELS_BOTTOM_LEVEL_SIZE * SURFELS_BOTTOM_LEVEL_SIZE)
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


	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<float> radii;

	memory_limitation_octree_size = 0;
	memory_limitation_count_bottom_size = 0;
	memory_limitation_bucket_size = 0;

	int sucessfull_inserts = 0;
	for (auto& s : surfels_)
	{
		//positions.push_back(s.mean);
		positions.push_back(s.get()->mean);
		normals.push_back(s.get()->normal);
		radii.push_back(s.get()->radius);

		auto i = scene_->get_irradiance_information(s.get()->mean, s.get()->normal, gi_primary_rays);
		colors.push_back(i.color);
		sucessfull_inserts += insert_surfel_into_octree(s.get()) ? 1 : 0;
	}

	dump_surfel_octree_to_gpu_memory();
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

void SurfelManagerOctree::update_surfels()
{
	if (surfels_.empty()) return;
	for (int i = 0; i < update_surfels_per_tick; i++)
	{
		if (last_updated_surfel >= surfels_.size())
		{
			last_updated_surfel = 0;
		}
		surfel* s = surfels_.at(last_updated_surfel).get();
		auto irradiance_info = scene_->get_irradiance_information(s->mean, s->normal, gi_primary_rays);
		s->diffuse_irradiance = s->diffuse_irradiance * static_cast<float>(s->samples) + irradiance_info.color *
			static_cast<float>(gi_primary_rays);
		s->samples += gi_primary_rays;
		s->diffuse_irradiance /= static_cast<float>(s->samples);
		update_surfel_data(s);
		last_updated_surfel++;
		if (last_updated_surfel >= surfels_.size())
		{
			last_updated_surfel = 0;
		}
	}

	//remove_surfel_from_octree(&surfels_.at(0));
	//surfels_.erase(surfels_.begin());
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

//TODO: test this with different radii 
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
