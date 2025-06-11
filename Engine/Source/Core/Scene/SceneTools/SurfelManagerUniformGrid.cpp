#include "SurfelManagerUniformGrid.h"

#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"
#include "../../Renderer/Texture/texture_buffer_object.h"

SurfelManagerUniformGrid::SurfelManagerUniformGrid(Scene* scene)
{
	scene_ = scene;
	surfels_texture_buffer_positions_ = new texture_buffer_object();
	surfels_texture_buffer_normals_ = new texture_buffer_object();
	surfels_texture_buffer_color_ = new texture_buffer_object();
	surfels_texture_buffer_radii_ = new texture_buffer_object();
	surfels_uniform_grid = new texture_buffer_object();
}

void SurfelManagerUniformGrid::clear_samples()
{
	samples_.clear();
}

void SurfelManagerUniformGrid::draw_ui()
{
	
	if (ImGui::Button("Generate surfels"))
	{
		recalculate_surfels();
	}
	ImGui::Checkbox("Draw debug tools", &draw_debug_tools_);
	
	if (draw_debug_tools_)
	{
		for (auto sample : samples_)
		{
			scene_->get_debug_tools()->draw_debug_point(sample.mean);
		}
	}

	ImGui::DragFloat("points per square meter", &points_per_square_meter);
	if (ImGui::Button("Recalculate"))
	{
		snap_samples_to_closest_surface();
	}
	ImGui::DragInt("surfel primary rays", &gi_primary_rays);
}



void SurfelManagerUniformGrid::snap_samples_to_closest_surface()
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
		samples_.push_back({point.position, point.normal, {1, 1, 1}, 1.0f/points_per_square_meter});
	}
}

std::vector<surfel> SurfelManagerUniformGrid::samples()
{
	return samples_;
}

void SurfelManagerUniformGrid::add_surfel_uniforms_to_shader(ShaderProgram* shader) const
{
	shader->addTexture(surfels_texture_buffer_color_, "surfels_texture_buffer_color_");
	shader->addTexture(surfels_texture_buffer_normals_, "surfels_texture_buffer_normals_");
	shader->addTexture(surfels_texture_buffer_positions_, "surfels_texture_buffer_positions_");
	shader->addTexture(surfels_texture_buffer_radii_, "surfels_texture_buffer_radii_");
	shader->addTexture(surfels_uniform_grid, "surfels_uniform_grid");
}


void SurfelManagerUniformGrid::init_surfels_buffer()
{
	if (has_surfels_buffer_) {
		scene_->get_global_context()->logger->print_warning("Scene already initialized surfel buffers");
		return;
	}

	scene_->get_global_context()->logger->print_info("Initializing surfel buffer...");
	scene_->get_global_context()->logger->print_info(std::format("Surfel buffer size : {}", SURFEL_BUFFER_AMOUNT));

	
	surfels_uniform_grid->init_u_int_2(SURFELS_GRID_SIZE*SURFELS_GRID_SIZE*SURFELS_GRID_SIZE);
	surfels_texture_buffer_positions_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_normals_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_color_->init_vec3(SURFEL_BUFFER_AMOUNT);
	surfels_texture_buffer_radii_->init_float(SURFEL_BUFFER_AMOUNT);
	scene_->get_global_context()->logger->print_info(std::format("Surfel buffer total memory : {}",
		sizeof(glm::vec3) * SURFEL_BUFFER_AMOUNT * 3 + sizeof(float) * SURFEL_BUFFER_AMOUNT)
		);
	scene_->get_global_context()->logger->print_info(std::format("Surfel grid total memory : {}",
	sizeof(unsigned int) * 2 * SURFELS_GRID_SIZE*SURFELS_GRID_SIZE*SURFELS_GRID_SIZE)
	);
	
	has_surfels_buffer_ = true;
	
}

void SurfelManagerUniformGrid::recalculate_surfels()
{

	if (!has_surfels_buffer_)
	{
		init_surfels_buffer();
	}
	
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec3> colors;
	std::vector<float> radii;

	unsigned int size = 0;

	for (auto s : samples_)
	{
		//positions.push_back(s.mean);
		positions.push_back(s.mean);
		normals.push_back(s.normal);
		radii.push_back(s.radius);

		auto i = scene_->get_irradiance_information(s.mean, s.normal, gi_primary_rays, s.radius);
		colors.push_back(i.color);
		size++;
	}
	

	std::map<unsigned int ,std::vector<unsigned int>> buckets;
	
	
	//sort surfels into corresponding bucket
	for (unsigned int i = 0; i < size; i++)
	{
		auto p = positions[i];
		auto n = normals[i];
		unsigned int buckets_for_surfle[8] = {};
		unsigned int buckets_length = get_surfel_buckets_from_ws_pos(p,n, buckets_for_surfle);
		//unsigned int bucket = get_surfel_bucket_from_ws_pos(p);

		for (int x = 0; x < buckets_length; x++)
		{
			auto bucket = buckets_for_surfle[x];
			if (buckets.contains(bucket))
			{
				buckets[bucket].push_back(i);
			} else
			{
				buckets[bucket] = std::vector<unsigned int>{i};
			}
		}

	}

	std::vector<glm::vec3> positions_surfel_stack;
	std::vector<glm::vec3> normals_surfel_stack;
	std::vector<glm::vec3> colors_surfel_stack;
	std::vector<float> radii_surfel_stack;


	
	unsigned int surfel_stack_counter = 0;
	for (auto bucket : buckets)
	{
		auto bucket_pos_in_grid = bucket.first;
		std::vector<unsigned int> bucket_surfel_ids = bucket.second;
		auto data = std::vector<glm::u32vec2>{glm::u32vec2(surfel_stack_counter,static_cast<unsigned>(bucket_surfel_ids.size()))};
		surfels_uniform_grid->update_u_int_2(&data, bucket_pos_in_grid);
		for (auto surfel_ids : bucket.second)
		{
			positions_surfel_stack.push_back(positions.at(surfel_ids));
			normals_surfel_stack.push_back(normals.at(surfel_ids));
			colors_surfel_stack.push_back(colors.at(surfel_ids));
			radii_surfel_stack.push_back(radii.at(surfel_ids));
			surfel_stack_counter++;
		}
	}
	
	bool success = true;
	success&= surfels_texture_buffer_positions_->update_vec3(&positions_surfel_stack, 0);
	success&= surfels_texture_buffer_normals_->update_vec3(&normals_surfel_stack, 0);
	success&= surfels_texture_buffer_color_->update_vec3(&colors_surfel_stack, 0);
	success&= surfels_texture_buffer_radii_->update_float(&radii_surfel_stack, 0);
	if (success)
	{
		scene_->get_global_context()->logger->print_info("surfels updated");

	} else
	{
		scene_->get_global_context()->logger->print_warning("surfels couldn't been updated correctly");
	}

}

//TODO : currently the surfle is considered a circle and is therefor placed in buckets it doesent belong to,
//this doesent affect visuals but performance 
unsigned int SurfelManagerUniformGrid::get_surfel_buckets_from_ws_pos(glm::vec3 ws_pos, glm::vec3 ws_normal, unsigned int buckets[8])
{
	buckets[0] = get_surfel_bucket_from_ws_pos(ws_pos);
	auto surfel_bucket_center = get_surfel_bucket_center(ws_pos);
	auto center_pos_v = ws_pos - surfel_bucket_center;
	center_pos_v.x = center_pos_v.x > 0 ? 1.0f : -1.0f;
	center_pos_v.y = center_pos_v.y > 0 ? 1.0f : -1.0f;
	center_pos_v.z = center_pos_v.z > 0 ? 1.0f : -1.0f;
	center_pos_v = normalize(center_pos_v);

	StructBoundingSphere b = {ws_pos, SURFELS_BUCKET_SIZE * 0.5f};
	StructBoundingBox bb = {};
	const glm::vec3 component_multiplier[8] = {
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(1.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(1.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 1.0f, 1.0f),
		glm::vec3(1.0f, 1.0f, 1.0f),
	};
	unsigned int filled_buckets = 1;
	
	auto half_size = glm::vec3(0.5) * SURFELS_BUCKET_SIZE;
	for (int i = 0 ; i < 7; i++)
	{
		auto center_neighbour = surfel_bucket_center + glm::normalize(center_pos_v * component_multiplier[i]) * SURFELS_BUCKET_SIZE;
		bb.min = center_neighbour - half_size;
		bb.max = center_neighbour + half_size;
		if (BoundingBoxHelper::are_overlapping(&bb, &b))
		{
			buckets[filled_buckets] = get_surfel_bucket_from_ws_pos(center_neighbour);
			filled_buckets++;
			
		}
	}
	return filled_buckets;
}


glm::vec3 SurfelManagerUniformGrid::get_surfel_bucket_center(glm::vec3 ws_pos) const
{
	ws_pos /= SURFELS_BUCKET_SIZE;
	ws_pos = glm::floor(ws_pos) + 0.5f ;
	return  ws_pos * SURFELS_BUCKET_SIZE;
}

unsigned int SurfelManagerUniformGrid::get_surfel_bucket_from_ws_pos(glm::vec3 ws_pos) const
{
	ws_pos /= SURFELS_BUCKET_SIZE;
	ws_pos += SURFELS_GRID_SIZE*0.5f;
	ws_pos = glm::floor(ws_pos);
	return ws_pos.x + SURFELS_GRID_SIZE * ws_pos.y + SURFELS_GRID_SIZE * (SURFELS_GRID_SIZE * ws_pos.z);
}
