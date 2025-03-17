#pragma once
#include <glm.hpp>
#include <vec4.hpp>

#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/CommonDataStructures/collider_intersection.h"
#include "../Core/CommonDataStructures/ray_cast_result.h"
#include "../Core/Scene/Object3D.h"


class RayIntersectionHelper
{
public:
	static struct_intersection* ray_plane_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction,
	                                                   glm::vec3 point_on_plane,
	                                                   glm::vec3 plane_normal);
	static void ray_plane_intersection(struct_intersection* intersection, glm::vec3 ray_origin, glm::vec3 ray_direction,
	                                   glm::vec3 point_on_plane, glm::vec3 plane_normal);

	static bool ray_bounding_box_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction,
	                                          StructBoundingBox* bounding_box);

	static glm::vec4 proyect_point_on_line(glm::vec3 a, glm::vec3 b, glm::vec3 p);

	static bool sphere_triangle_intersection(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 circle_pos,
	                                         float circle_radius);

	static void get_closest_point_on_triangle(glm::vec3 v_0, glm::vec3 v_1, glm::vec3 v_2,
	                                          glm::vec3 circle_pos, ray_cast_result* result);
};
