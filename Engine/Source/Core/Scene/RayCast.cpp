#include "RayCast.h"

#include <chrono>
#include <gtx/string_cast.hpp>

#include "Collider.h"
#include "Mesh3D.h"


ray_cast_hit RayCast::ray_cast(Scene* scene_context, std::string* collision_tag, glm::vec3 ray_cast_origin,
                               glm::vec3 ray_cast_direction,
                               float length,
                               bool ignore_back_face)
{
	auto start = std::chrono::system_clock::now();
	//TODO: this can and has to optimized a lot by stacking bounding boxes
	//also do this with bounding box objects and not with regular geometry
	auto cast_hit = ray_cast_hit();
	recurse_scene_model_ray_cast(&cast_hit, collision_tag, scene_context->get_root(), ray_cast_origin,
	                             normalize(ray_cast_direction), length, ignore_back_face);
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;
	std::cout << "raycast time total: " << elapsed_seconds.count() << "s\n";
	return cast_hit;
}


void RayCast::recurse_scene_model_ray_cast(ray_cast_hit* ray_cast_hit, std::string* collision_tag, Object3D* object,
                                           glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction_normalized,
                                           double length, bool ignore_back_face)
{
	if (object->has_tag(*collision_tag) && object->visible)
	{
		auto mesh_collider = dynamic_cast<Collider*>(object);
		mesh_collider->check_collision_ws(ray_cast_origin, ray_cast_direction_normalized, length, ignore_back_face,
		                                  ray_cast_hit);
	}
	if (ray_cast_hit->hit)
	{
		return;
	}
	for (auto child : object->get_children())
	{
		recurse_scene_model_ray_cast(ray_cast_hit, collision_tag, child, ray_cast_origin, ray_cast_direction_normalized,
		                             length,
		                             ignore_back_face);
	}
}

bool RayCast::recurse_proximity_check(Object3D* object, std::string* collision_tag, glm::vec3 proximity_center,
                                      float radius)
{
	if (object->has_tag(*collision_tag) && object->visible)
	{
		auto mesh_collider = dynamic_cast<Collider*>(object);
		if (mesh_collider->is_in_proximity(proximity_center, radius))
		{
			return true;
		}
	}
	for (auto child : object->get_children())
	{
		if (recurse_proximity_check(child, collision_tag, proximity_center, radius))
		{
			return true;
		}
	}
	return false;
}


//EDITOR ONLY RAY CAST FOR OBJECT SELECTION 

ray_cast_hit RayCast::ray_cast_editor(Scene* scene_context,
                                      glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction, bool ignore_back_face)
{
	auto cast_hit = ray_cast_hit();

	std::string tag = "ENGINE_COLLIDER";
	recurse_scene_model_ray_cast(&cast_hit, &tag, scene_context->get_root(), ray_cast_origin,
	                             normalize(ray_cast_direction), 100000.0, ignore_back_face);

	return cast_hit;
}
