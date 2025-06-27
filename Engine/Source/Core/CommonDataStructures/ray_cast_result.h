#pragma once
#include <vec3.hpp>

class Object3D;

struct ray_cast_result
{
	bool hit = false; //if the raycast hit something
	bool hit_back_face = false; //if the intersected surface was a backface
	double distance_from_origin = std::numeric_limits<double>::max();
	Object3D* object_3d = nullptr; //the object that was hit
	glm::vec3 hit_world_space = {0, 0, 0}; //hit intersection in world space
	glm::vec3 hit_normal_world_space = {0, 0, 0}; //normal of the hit
	glm::vec3 hit_local = {0, 0, 0}; //hit intersection in world space
	glm::vec3 hit_normal_local = {0, 0, 0}; //normal of the hit
	unsigned int vertex_indices[3] = {0, 0, 0}; //indices of the vertices of the hit face
};

inline void copy_ray_cast_result(const ray_cast_result *copy_from, ray_cast_result *copy_to)
{
	copy_to->hit = copy_from->hit;
	copy_to->hit_back_face = copy_from->hit_back_face;
	copy_to->distance_from_origin = copy_from->distance_from_origin;
	copy_to->object_3d = copy_from->object_3d;
	copy_to->hit_world_space = copy_from->hit_world_space;
	copy_to->hit_normal_world_space = copy_from->hit_normal_world_space;
	copy_to->hit_local = copy_from->hit_local;
	copy_to->hit_normal_local = copy_from->hit_normal_local;
	copy_to->vertex_indices[0] = copy_from->vertex_indices[0];
	copy_to->vertex_indices[1] = copy_from->vertex_indices[1];
	copy_to->vertex_indices[2] = copy_from->vertex_indices[2];
}
