#include "RayIntersectionHelper.h"

#include <detail/func_geometric.inl>

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
