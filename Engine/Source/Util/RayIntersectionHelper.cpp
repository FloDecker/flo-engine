#include "RayIntersectionHelper.h"

#include <detail/func_geometric.inl>

#define in_bounding_box(a, min,max) ((a)<(max)&&(a)>(min))

Intersection* RayIntersectionHelper::RayPlaneIntersection(glm::vec3 ray_origin, glm::vec3 ray_direction_normalized,
                                                          glm::vec3 point_on_plane, glm::vec3 plane_normal)
{
    auto intersection = new Intersection;
    RayPlaneIntersection(intersection, ray_origin, ray_direction_normalized, point_on_plane, plane_normal);
    return intersection;
}

void RayIntersectionHelper::RayPlaneIntersection(Intersection* intersection, glm::vec3 ray_origin,
                                                 glm::vec3 ray_direction_normalized, glm::vec3 point_on_plane,
                                                 glm::vec3 plane_normal)
{
    if (glm::dot(plane_normal, ray_direction_normalized) == 0.0)
    {
        intersection->intersected = false;
        return;
    }
    float d = -glm::dot(plane_normal, point_on_plane);
    float t =
        -((glm::dot(plane_normal, ray_origin) + d) /
            glm::dot(plane_normal, ray_direction_normalized));

    intersection->intersection_point = ray_origin + t * ray_direction_normalized;
    intersection->intersected = true;
}

//Ray and bounding box have to be in the same coordainte system
bool RayIntersectionHelper::ray_bounding_box_intersection(glm::vec3 ray_origin,
    glm::vec3 ray_direction, StructBoundingBox* bounding_box)
{
    bool first_intersection = false;
    auto intersection = new Intersection;

    //max
    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->max,{1,0,0});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.y,bounding_box->min.y,bounding_box->max.y) &&
        in_bounding_box(intersection->intersection_point.z,bounding_box->min.z,bounding_box->max.z)
        ) return true;
    
    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->max,{0,1,0});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.x,bounding_box->min.x,bounding_box->max.x) &&
        in_bounding_box(intersection->intersection_point.z,bounding_box->min.z,bounding_box->max.z)
        ) return true;

    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->max,{0,0,1});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.x,bounding_box->min.x,bounding_box->max.x) &&
        in_bounding_box(intersection->intersection_point.y,bounding_box->min.y,bounding_box->max.y)
        ) return true;

    //min
    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->min,{1,0,0});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.y,bounding_box->min.y,bounding_box->max.y) &&
        in_bounding_box(intersection->intersection_point.z,bounding_box->min.z,bounding_box->max.z)
        ) return true;
    
    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->min,{0,1,0});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.x,bounding_box->min.x,bounding_box->max.x) &&
        in_bounding_box(intersection->intersection_point.z,bounding_box->min.z,bounding_box->max.z)
        ) return true;

    intersection->intersected = false;
    RayPlaneIntersection(intersection, ray_origin,ray_direction,
        bounding_box->min,{0,0,1});
    if (intersection->intersected &&
        in_bounding_box(intersection->intersection_point.x,bounding_box->min.x,bounding_box->max.x) &&
        in_bounding_box(intersection->intersection_point.y,bounding_box->min.y,bounding_box->max.y)
        ) return true;

    return false;    
}
