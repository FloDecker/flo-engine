#pragma once
#include "Scene.h"
#include "../CommonDataStructures/ray_cast_result.h"
#include "../../Util/RayIntersectionHelper.h"

class RayCast
{
public:
	//ray cast in scene
	static ray_cast_result ray_cast(
		Scene* scene_context,
		glm::vec3 ray_cast_origin,
		glm::vec3 ray_cast_direction,
		float length,
		bool ignore_back_face = true
	);

	//TODO this shouldn't be used by the game developer !Engine only!
	static ray_cast_result ray_cast_editor(
		Scene* scene_context,
		glm::vec3 ray_cast_origin,
		glm::vec3 ray_cast_direction,
		bool ignore_back_face = true);

private:
	static void recurse_scene_model_ray_cast(
		ray_cast_result* ray_cast_hit,
		Object3D* object,
		glm::vec3 ray_cast_origin,
		glm::vec3 ray_cast_direction_normalized,
		double length,
		bool ignore_back_face = true
	);

	static void recurse_proximity_check(
		ray_cast_result* result,
		Object3D* object,
		glm::vec3 proximity_center, float radius
	);
};
