#include "SurfelManagerOctree.h"

#include <fstream>
#include <random>
#include <utility>
#include <GLFW/glfw3.h>
#include <gtx/string_cast.hpp>

#include "../Camera3D.h"
#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../../Renderer/Shader/compute_shader.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"
#include "../../Renderer/Texture/texture_buffer_object.h"

SurfelManagerOctree::SurfelManagerOctree(Scene* scene)
{
	scene_ = scene;

	surfel_ssbo_consumer_ = new ssbo_double_buffer<surfel_gpu>();
	surfel_ssbo_consumer_->init(SURFELS_BUCKET_AMOUNT * SURFEL_BUCKET_SIZE_ON_GPU, 3, 4);
	surfel_ssbo_consumer_->bind_back(3);
	surfel_ssbo_consumer_->bind_front(4);

	surfels_octree_consumer_ = new ssbo_double_buffer<surfel_octree_element>();
	surfels_octree_consumer_->init(SURFEL_OCTREE_SIZE, 5, 6);
	surfels_octree_consumer_->bind_back(5);
	surfels_octree_consumer_->bind_front(6);

	surfel_allocation_metadata_ = new ssbo<::surfel_allocation_metadata>();
	surfel_allocation_metadata_->init_buffer_object(1, 2);
	surfel_allocation_metadata_->insert_data(new surfel_allocation_metadata(1, 1), 0);

	least_shaded_regions_ = new ssbo<glm::vec4>();
	least_shaded_regions_->init_buffer_object(4096, 7);

	updated_octree_positions_ = new ssbo<glm::uint>();
	updated_octree_positions_->init_buffer_object(40000, 8);

	insert_surfel_compute_shader = new compute_shader();
	insert_surfel_compute_shader->loadFromFile("EngineContent/ComputeShader/SurfelInserter.glsl");
	insert_surfel_compute_shader->compileShader();

	compute_shader_approximate_gi = new compute_shader();
	compute_shader_approximate_gi->loadFromFile("EngineContent/ComputeShader/SurfelAoApproximatorParallelized.glsl");
	compute_shader_approximate_gi->compileShader();

	compute_shader_find_least_shaded_pos = new compute_shader();
	compute_shader_find_least_shaded_pos->loadFromFile("EngineContent/ComputeShader/LowestPosFinder.glsl");
	compute_shader_find_least_shaded_pos->compileShader();

	compute_shader_sync_buffers = new compute_shader();
	compute_shader_sync_buffers->loadFromFile("EngineContent/ComputeShader/SyncBuffers.glsl");
	compute_shader_sync_buffers->compileShader();

	glGenQueries(1, &time_query_);
}

void SurfelManagerOctree::dump_metadata_history() const
{
	{
		std::ofstream outFile("meta_data.txt");
		if (!outFile)
		{
			std::cerr << "Error opening file!\n";
			return;
		}

		// Dump vector contents
		for (const auto& value : meta_data_history_)
		{
			outFile << value.first << "|" << value.second.debug_int_32 << "\n";
		}
		outFile.close();
	}

	{
		std::ofstream out_file("state_history.txt");
		if (!out_file)
		{
			std::cerr << "Error opening file!\n";
			return;
		}

		// Dump vector contents
		for (const auto& value : state_pass_time_history_)
		{
			out_file << value.time_stamp << "|" << value.render_time << "|" << value.state << "\n";
		}
		out_file.close();
	}
}

void SurfelManagerOctree::draw_ui()
{
	if (ImGui::Button("Clear Surfels on GPU"))
	{
		clear_surfels_on_gpu();
		recording_start_ = glfwGetTime();
		meta_data_history_.clear();
		last_sample = glfwGetTime() - sample_interval - 100;

		state_pass_time_history_.clear();
	}
	if (ImGui::Button("Dump metadata"))
	{
		dump_metadata_history();
	}

	if (ImGui::Checkbox("Update Surfels", &update_surfels_next_tick))
	{
		if (update_surfels_next_tick)
		{
			recording_start_ = glfwGetTime();
			last_sample = glfwGetTime() - sample_interval - 100;
			meta_data_history_.clear();

			state_pass_time_history_.clear();
		}
	}

	ImGui::SeparatorText("Surfel parameters");
	ImGui::DragFloat("Minimal size of a surfel", &surfel_parameters_.minimal_surfel_radius);
	ImGui::DragFloat("Surfel size multiplier post insertion", &surfel_parameters_.surfel_insert_size_multiplier);
	ImGui::DragFloat("Surfel insertion threshold", &surfel_parameters_.surfel_insertion_threshold);
	ImGui::DragInt("Pixels per surfel", &surfel_parameters_.pixels_per_surfel);


	ImGui::Separator();
	ImGui::Checkbox("Draw boxes on update nodes", &draw_debug_boxes_);
	ImGui::Checkbox("Record surfel metadata", &record_surfel_metadata);
	ImGui::DragInt("Surfel GI updates per tick", &surfel_gi_updates_per_tick);
	ImGui::DragFloat("Meta data sample interval", &sample_interval);

	::surfel_allocation_metadata allocation_metadata;
	if (!meta_data_history_.empty())
	{
		allocation_metadata = meta_data_history_.back().second;
	}
	ImGui::Text("allocation metadata %d", allocation_metadata.debug_int_32);
	ImGui::Text("octree bucket pointer %d", allocation_metadata.surfel_bucket_pointer);
	ImGui::Text("created octree nodes %d", allocation_metadata.surfel_octree_pointer);
}


void SurfelManagerOctree::generate_surfels_via_compute_shader() const
{
	if (camera_ != nullptr)
	{
		auto t = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0);
		if (t != nullptr)
		{
			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> float_dist_0_1(0.0f, 1.0f);
			insert_surfel_compute_shader->use();
			insert_surfel_compute_shader->set_uniform_vec3_f("camera_position",
			                                                 glm::value_ptr(camera_->getWorldPosition()));

			insert_surfel_compute_shader->setUniformMatrix4("projection_matrix",
			                                                glm::value_ptr(*camera_->get_camera()->getProjection()));
			insert_surfel_compute_shader->setUniformMatrix4("view_matrix",
			                                                glm::value_ptr(*camera_->get_camera()->getView()));

			insert_surfel_compute_shader->set_uniform_float("minimal_surfel_radius",
			                                                surfel_parameters_.minimal_surfel_radius);
			insert_surfel_compute_shader->set_uniform_float("surfel_insertion_threshold",
			                                                surfel_parameters_.surfel_insertion_threshold);
			insert_surfel_compute_shader->set_uniform_float("surfel_insert_size_multiplier",
			                                                surfel_parameters_.surfel_insert_size_multiplier);
			insert_surfel_compute_shader->setUniformInt("pixels_per_surfel", surfel_parameters_.pixels_per_surfel);

			auto offset = glm::vec3(float_dist_0_1(gen), float_dist_0_1(gen), 0.0);
			insert_surfel_compute_shader->set_uniform_vec3_f("random_offset",
			                                                 glm::value_ptr(offset));
			const unsigned int x_groups = t->width() / surfel_parameters_.pixels_per_surfel + 1;
			const unsigned int y_groups = t->height() / surfel_parameters_.pixels_per_surfel + 1;
			glDispatchCompute(x_groups, y_groups, 1);
		}
	}
}

void SurfelManagerOctree::copy_update_positions_to_cpu()
{
	const auto least_shaded_regions = static_cast<glm::vec4*>(least_shaded_regions_->write_ssbo_to_cpu());

	const auto width = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->width() / 32 + 1;
	const auto height = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->height() / 32 + 1;

	ssbo<glm::vec4*>::unmap();

	std::set<std::pair<unsigned, std::array<unsigned, 3>>> sample_positons_set = {};

	for (int j = 0; j < width * height; j++)
	{
		glm::vec4 c = least_shaded_regions[j];
		if (c.w < 0)
		{
			continue;
		}
		const int level = c.w;
		const auto bucket_coordinates = get_bucket_coordinates_from_ws(glm::vec3(c), level);
		auto bucket_pair = std::pair<unsigned, std::array<unsigned, 3>>(
			static_cast<unsigned>(level), {bucket_coordinates.x, bucket_coordinates.y, bucket_coordinates.z}
		);
		sample_positons_set.insert(bucket_pair);
	}
	sample_positons_ = std::vector(sample_positons_set.begin(), sample_positons_set.end());
}

bool SurfelManagerOctree::update_surfels_in_update_queue(int amount)
{
	const auto queue_size = static_cast<int>(sample_positons_.size());
	amount = std::min(amount, queue_size);

	for (int i = 0; i < amount; i++)
	{
		auto [level, coordinate] = sample_positons_.back();
		const glm::uvec3 coordinates = {coordinate[0], coordinate[1], coordinate[2]};
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
	constexpr int update_patch_dimension = 32;
	const int width = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->width() /
		update_patch_dimension + 1;
	const int height = camera_->get_camera()->get_render_target()->get_color_attachment_at_index(0)->height() /
		update_patch_dimension + 1;
	compute_shader_find_least_shaded_pos->use();
	glDispatchCompute(width, height, 1);
}

void SurfelManagerOctree::compute_shader_ao_approximation(uint32_t level, glm::uvec3 pos_in_octree) const
{
	compute_shader_approximate_gi->use();
	compute_shader_approximate_gi->setUniformInt("offset_id", 0);
	compute_shader_approximate_gi->setUniformInt("calculation_level", level);
	compute_shader_approximate_gi->set_uniform_vec3_u("pos_ws_start", value_ptr(pos_in_octree));
	glDispatchCompute(SURFEL_BUCKET_SIZE_ON_GPU, 1, 1);
}

void SurfelManagerOctree::sync_buffers() const
{
	const auto allocation_data = static_cast<struct::surfel_allocation_metadata*>(surfel_allocation_metadata_->
		write_ssbo_to_cpu());
	surfel_allocation_metadata_->unmap();
	const auto changed_buckets_amount = allocation_data->octree_pointer_update_index;
	compute_shader_sync_buffers->use();
	glDispatchCompute(changed_buckets_amount, 1, 1);
}

glm::uvec3 SurfelManagerOctree::get_bucket_coordinates_from_ws(glm::vec3 ws_pos, int level) const
{
	auto node_size = node_size_at_level(level);
	auto p = glm::uvec3(floor((ws_pos + octree_total_extension * 0.5f) / node_size));
	return glm::clamp(p, glm::uvec3(0), glm::uvec3(get_bucket_amount_at_level(level)));
}


glm::vec3 SurfelManagerOctree::get_ws_bucket_lowest_edge_from_octree_index(int layer, glm::uvec3 octree_index) const
{
	return glm::vec3(-octree_total_extension * 0.5f) + glm::vec3(octree_index) * node_size_at_level(layer);
}

unsigned int SurfelManagerOctree::get_bucket_amount_at_level(unsigned int level)
{
	return 1u << level;
}

float SurfelManagerOctree::node_size_at_level(unsigned int level) const
{
	return octree_total_extension / static_cast<float>(1 << level);
}


void SurfelManagerOctree::record_state_time(double time, double delta, int state)
{
	if (update_surfels_next_tick)
	{
		state_history s = {.state = state, .time_stamp = time - recording_start_, .render_time = delta};
		state_pass_time_history_.push_back(s);
	}
}

void SurfelManagerOctree::clear_surfels_on_gpu()
{
	surfel_allocation_metadata_->insert_data(new surfel_allocation_metadata(1, 1), 0);

	surfel_ssbo_consumer_->clear();
	surfels_octree_consumer_->clear();

	manager_state_ = 0;
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
	insert_surfel_compute_shader->addTexture(surfel_framebuffer_metadata_0_texture, "surfel_framebuffer_metadata_0");
	insert_surfel_compute_shader->addTexture(camera->get_scene()->get_scene_direct_light()->light_map(),
	                                         "direct_light_map_texture");


	compute_shader_find_least_shaded_pos->addTexture(t_pos, "gPos");
	compute_shader_find_least_shaded_pos->addTexture(surfel_framebuffer_metadata_0_texture,
	                                                 "surfel_framebuffer_metadata_0");
	compute_shader_find_least_shaded_pos->addTexture(surfel_framebuffer_metadata_1_texture,
	                                                 "surfel_framebuffer_metadata_1");
	compute_shader_find_least_shaded_pos->addTexture(surfel_framebuffer_texture, "surfel_framebuffer");
}

void SurfelManagerOctree::tick(double time_stamp)
{
	if (!update_surfels_next_tick)
	{
		return;
	}

	if (record_surfel_metadata && time_stamp - last_sample > sample_interval)
	{
		last_sample = time_stamp;
		struct::surfel_allocation_metadata* allocation_metadata = static_cast<struct::surfel_allocation_metadata*>(
			surfel_allocation_metadata_->write_ssbo_to_cpu());
		meta_data_history_.emplace_back(time_stamp - recording_start_, *allocation_metadata);
		ssbo<::surfel_allocation_metadata>::unmap();
	}

	if (compute_fence_state_machine_)
	{
		GLenum result = glClientWaitSync(compute_fence_state_machine_, 0, 0);

		if (result == GL_ALREADY_SIGNALED || result == GL_CONDITION_SATISFIED)
		{
			glDeleteSync(compute_fence_state_machine_);
			compute_fence_state_machine_ = nullptr;
		}
		else
		{
			return;
		}
	}

	GLuint available = 0;
	glGetQueryObjectuiv(time_query_, GL_QUERY_RESULT_AVAILABLE, &available);
	if (record_surfel_metadata && available)
	{
		GLuint64 ns = 0;
		glGetQueryObjectui64v(time_query_, GL_QUERY_RESULT, &ns); // nanoseconds
		double ms = ns / 1.0e6;
		record_state_time(time_stamp, ms, last_state_);
	}
	glBeginQuery(GL_TIME_ELAPSED, time_query_);
	last_state_ = manager_state_;
	switch (manager_state_)
	{
	case 0: //SYNC BUFFERS
		sync_buffers();
		compute_fence_state_machine_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 1;
		break;
	case 1: //INSERT SURFELS
		generate_surfels_via_compute_shader();
		compute_fence_state_machine_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 2;
		break;
	case 2: //FIND UPDATE POSITIONS
		find_best_world_positions_to_update_lighting();
		compute_fence_state_machine_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		manager_state_ = 3;
		last_updated_regions_available_ = true;
		break;
	case 3: // GI UPDATE
		if (last_updated_regions_available_)
		{
			copy_update_positions_to_cpu();
			last_updated_regions_available_ = false;
		}
		if (update_surfels_in_update_queue(surfel_gi_updates_per_tick))
		{
			manager_state_ = 4;
		}
		compute_fence_state_machine_ = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
		break;
	case 4:
		swap_surfel_buffers(); //swap front and backbuffer
		manager_state_ = 0;
		break;
	default:
		manager_state_ = 0;
	}
	glEndQuery(GL_TIME_ELAPSED);
}

void SurfelManagerOctree::swap_surfel_buffers() const
{
	surfel_ssbo_consumer_->swap();
	surfels_octree_consumer_->swap();
}
