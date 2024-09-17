#pragma once
#include <vec3.hpp>
#include <vec4.hpp>

#include "../Core/CommonDataStructures/StructBoundingBox.h"
#include "../Core/Scene/Object3D.h"

struct RayCastHit
{
	bool hit = false; //if the raycast hit something
	double distance_from_origin;
	Object3D* object_3d; //the object that was hit
	glm::vec3 hit_world_space; //hit intersection in world space
	glm::vec3 hit_normal_world_space; //normal of the hit
	glm::vec3 hit_local; //hit intersection in world space
	glm::vec3 hit_normal_local; //normal of the hit
};
struct Intersection
{
    bool intersected = false;
    glm::vec3 intersection_point = {0, 0, 0};
};

class RayIntersectionHelper
{
public:
    static Intersection* RayPlaneIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction, glm::vec3 point_on_plane,
                                              glm::vec3 plane_normal);
    static void RayPlaneIntersection(Intersection* intersection, glm::vec3 ray_origin, glm::vec3 ray_direction,
                                     glm::vec3 point_on_plane, glm::vec3 plane_normal);
    
    static bool ray_bounding_box_intersection(glm::vec3 ray_origin, glm::vec3 ray_direction,
                                              StructBoundingBox* bounding_box);

    static glm::vec4 proyect_point_on_line(glm::vec3 a, glm::vec3 b, glm::vec3 p);

    static bool sphere_triangle_intersection(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 circle_pos,
                                                  float circle_radius);
	
    static bool point_triangle_proximity_check(glm::vec3 v_0, glm::vec3 v_1, glm::vec3 v_2,
                                                               glm::vec3 circle_pos, float radius);

};
