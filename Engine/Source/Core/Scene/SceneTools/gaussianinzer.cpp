#include "gaussianinzer.h"

#include "../RayCast.h"
#include "../Scene.h"

void gaussianinzer::on_transform_changed()
{
	get_sample_positions_sparse();
}

void gaussianinzer::clear_samples()
{
	for (auto s : samples_)
	{
		if (s.gaussian_at_sample != nullptr)
		{
			free(s.gaussian_at_sample);
		}
	}
	samples_.clear();
}

void gaussianinzer::calculate_gaussian()
{
	for (auto s : samples_)
	{
		
	}
}

void gaussianinzer::get_sample_positions_sparse()
{
	clear_samples();
	glm::vec3 lower_left = transform_vertex_to_world_space({-1,-1,-1});
	glm::vec3 upper_right = transform_vertex_to_world_space({1,1,1});
	
	unsigned int samples_in_x = static_cast<unsigned int>(upper_right.x - lower_left.x) * samples_per_meter;
	unsigned int samples_in_y = static_cast<unsigned int>(upper_right.y - lower_left.y) * samples_per_meter;
	unsigned int samples_in_z = static_cast<unsigned int>(upper_right.z - lower_left.z) * samples_per_meter;

	for (unsigned int x = 0; x < samples_in_x; x++)
	{
		for (unsigned int y = 0; y < samples_in_y; y++)
		{
			for (unsigned int z = 0; z < samples_in_z; z++)
			{
				auto a = glm::vec3(x,y,z);
				auto b = glm::vec3(samples_in_x,samples_in_y,samples_in_z);
				auto p = transform_vertex_to_world_space(glm::vec3(-1,-1,-1) + a/b * 2.0f);
				scene_->get_debug_tools()->draw_debug_point(p);
				samples_.push_back({p});
			}
		}
	}

	
}
