#include "gaussianizer.h"

#include "../RayCast.h"
#include "../Scene.h"
#include "../../../Util/BoundingBoxHelper.h"
#include "../Modifiers/Implementations/Colliders/collider_modifier.h"

void gaussianizer::on_transform_changed()
{
	glm::vec3 lower_left = transform_vertex_to_world_space({-1, -1, -1});
	glm::vec3 upper_right = transform_vertex_to_world_space({1, 1, 1});
	auto bb = StructBoundingBox(lower_left, upper_right);
	scene_->get_debug_tools()->draw_debug_cube(BoundingBoxHelper::get_center_of_bb(&bb), 0, glm::quat(),
	                                           BoundingBoxHelper::get_scale_of_bb(&bb));
}

void gaussianizer::clear_samples()
{
	samples_.clear();
}

void gaussianizer::calculate_gaussian()
{
	for (auto s : samples_)
	{
	}
}

void gaussianizer::draw_object_specific_ui()
{
	ImGui::Checkbox("Draw debug tools", &draw_debug_tools_);
	if (draw_debug_tools_)
	{
		for (auto sample : samples_)
		{
			scene_->get_debug_tools()->draw_debug_point(sample.mean);
		}
	}

	ImGui::DragInt("points per square meter", &points_per_square_meter);
	if (ImGui::Button("Recalculate"))
	{
		snap_samples_to_closest_surface();
	}
}

void gaussianizer::snap_samples_to_closest_surface()
{
	clear_samples();

	glm::vec3 lower_left = transform_vertex_to_world_space({-1, -1, -1});
	glm::vec3 upper_right = transform_vertex_to_world_space({1, 1, 1});
	StructBoundingBox boundingbox = {lower_left, upper_right};
	auto colliders_in_bb = std::vector<collider_modifier*>();
	scene_->get_colliders_in_bounding_box(&boundingbox, VISIBILITY, &colliders_in_bb);

	auto points = std::vector<vertex>();
	for (collider_modifier* collider : colliders_in_bb)
	{
		collider->scatter_points_on_surface(&points, points_per_square_meter);
	}

	for (auto point : points)
	{
		samples_.push_back({point.position, point.normal, {1, 1, 1}, 1.0});
	}
}

std::vector<gaussian> gaussianizer::samples() const
{
	return samples_;
}
