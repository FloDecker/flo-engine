#pragma once
#include <vec3.hpp>

class Object3D;
struct ray_cast_result
{
	bool hit = false; //if the raycast hit something
	double distance_from_origin = std::numeric_limits<double>::max();
	Object3D* object_3d = nullptr; //the object that was hit
	glm::vec3 hit_world_space = {0, 0, 0}; //hit intersection in world space
	glm::vec3 hit_normal_world_space = {0, 0, 0}; //normal of the hit
	glm::vec3 hit_local = {0, 0, 0}; //hit intersection in world space
	glm::vec3 hit_normal_local = {0, 0, 0}; //normal of the hit
	unsigned int vertex_indices[3] = {0, 0, 0}; //indices of the vertices of the hit face
};
