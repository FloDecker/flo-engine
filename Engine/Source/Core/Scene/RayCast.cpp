#include "RayCast.h"

#include <chrono>
#include <gtx/string_cast.hpp>

#include "Primitive3D/Mesh3D.h"
#include "../Scene/Modifiers/Implementations/Colliders/collider_modifier.h"
#include "SceneTools/VoxelizerTools/AbstractVoxelizer.h"

ray_cast_result RayCast::ray_cast(Scene* scene_context, glm::vec3 ray_cast_origin,
                                  glm::vec3 ray_cast_direction,
                                  float length,
                                  bool ignore_back_face)
{
	auto start = std::chrono::system_clock::now();
	//TODO: this can and has to optimized a lot by stacking bounding boxes
	//also do this with bounding box objects and not with regular geometry
	auto cast_hit = ray_cast_result();
	recurse_scene_model_ray_cast(&cast_hit, scene_context->get_root(), ray_cast_origin,
	                             normalize(ray_cast_direction), length, ignore_back_face);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "raycast time total: " << elapsed_seconds.count() << "s\n";
	return cast_hit;
}


void RayCast::recurse_scene_model_ray_cast(ray_cast_result* ray_cast_hit, Object3D* object,
                                           glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction_normalized,
                                           double length, bool ignore_back_face)
{
	const auto colliders = object->get_modifiers_by_id(collider_modifier::MODIFIER_ID);

	for (const auto obj : colliders)
	{
		auto collider = dynamic_cast<collider_modifier*>(obj);
		collider->ray_intersection_world_space(ray_cast_origin, ray_cast_direction_normalized, length, ignore_back_face,
		                                       ray_cast_hit);
		if (ray_cast_hit->hit)
		{
			return;
		}
	}

	for (auto child : object->get_children())
	{
		recurse_scene_model_ray_cast(ray_cast_hit, child, ray_cast_origin, ray_cast_direction_normalized,
		                             length,
		                             ignore_back_face);
	}
}

void RayCast::recurse_proximity_check(ray_cast_result* result, Object3D* object,
                                      glm::vec3 proximity_center,
                                      float radius)
{
	const auto colliders = object->get_modifiers_by_id(collider_modifier::MODIFIER_ID);
	for (const auto obj : colliders)
	{
		auto collider = dynamic_cast<collider_modifier*>(obj);
		collider->is_in_proximity(proximity_center, radius, result);
		if (result->hit)
		{
			return;
		}
	}
	for (auto child : object->get_children())
	{
		recurse_proximity_check(result, child, proximity_center, radius);
	}
}


//EDITOR ONLY RAY CAST FOR OBJECT SELECTION 

ray_cast_result RayCast::ray_cast_editor(Scene* scene_context,
                                         glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction, bool ignore_back_face)
{
	auto cast_hit = ray_cast_result();

	recurse_scene_model_ray_cast(&cast_hit, scene_context->get_root(), ray_cast_origin,
	                             normalize(ray_cast_direction), 100000.0, ignore_back_face);

	return cast_hit;
}
