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

//return vector.w is 1 if the result is clamped to the points and 0 if its in between a and b
glm::vec4 RayIntersectionHelper::proyect_point_on_line(glm::vec3 a, glm::vec3 b, glm::vec3 p)
{
    float distance_ab = glm::distance(a,b);
    glm::vec3 a_b_n = glm::normalize(b-a);
    glm::vec3 a_p = p-a;
    float d = glm::dot(a_p, a_b_n);
    if (d < 0 )
    {
        return{a,1};
    }
    if (d > distance_ab )
    {
        return{b,1};
    }

    return {a + a_b_n * d,0};
    
}

bool RayIntersectionHelper::sphere_triangle_intersection(glm::vec3 a, glm::vec3 b, glm::vec3 c,
    glm::vec3 circle_pos, float circle_radius)
{
    auto a_b_ = proyect_point_on_line(a,b,circle_pos);
    auto b_c_ = proyect_point_on_line(b,c,circle_pos);
    auto c_a_ = proyect_point_on_line(c,a,circle_pos);


    return (
        glm::distance(glm::vec3(a_b_),circle_pos) < circle_radius ||
        glm::distance(glm::vec3(b_c_),circle_pos) < circle_radius ||
        glm::distance(glm::vec3(c_a_),circle_pos) < circle_radius );
    
    
}
