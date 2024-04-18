#include "RayCast.h"

#include <gtx/string_cast.hpp>

#include "Collider.h"
#include "Mesh3D.h"


RayCastHit RayCast::ray_cast(SceneContext* scene_context, glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction,
                             float length,
                             bool ignore_back_face)
{
    //TODO: this can and has to optimized a lot by stacking bounding boxes
    //also do this with bounding box objects and not with regular geometry
    RayCastHit ray_cast_hit = RayCastHit{
        false,
        length + 1.0,
        nullptr,
        glm::vec3(),
        glm::vec3()
    };
    recurse_scene_model_ray_cast(&ray_cast_hit, scene_context->get_root(), ray_cast_origin,
                                 glm::normalize(ray_cast_direction), length, ignore_back_face);
    return ray_cast_hit;
}

void RayCast::recurse_scene_model_ray_cast(RayCastHit* ray_cast_hit, Object3D* object,
                                           glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction_normalized,
                                           double length, bool ignore_back_face)
{
    if (object->has_tag("ENGINE_COLLIDER"))
    {
        auto mesh_collider = dynamic_cast<MeshCollider*>(object);
        geometry_ray_cast(ray_cast_hit, mesh_collider->get_vertex_arrays(), mesh_collider->getGlobalTransform(), ray_cast_origin,
                          ray_cast_direction_normalized, length, ignore_back_face);
    }
    for (auto child : object->get_children())
    {
        recurse_scene_model_ray_cast(ray_cast_hit, child, ray_cast_origin, ray_cast_direction_normalized, length,
                                     ignore_back_face);
    }
}


//ray cast logic on geometry level, can be invoked for an array of vertex array with the same global transform (normally one object with different vertex arrays)
void RayCast::geometry_ray_cast(
    RayCastHit* ray_cast_hit,
    std::vector<struct_vertex_array*>* vertex_arrays_of_geometry,
    glm::mat4 global_transform,
    glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction_normalized,
    double length, bool ignore_back_face)
{
    auto global_inverse = glm::inverse(global_transform);
    std::cout << glm::to_string(global_transform)<< "\n";
    std::cout<< "raycast origin GLOBAL " << ray_cast_origin.x <<"."<<ray_cast_origin.y <<"."<<ray_cast_origin.z <<".\n";
    glm::vec4 ray_cast_origin_vec4_local = global_inverse * glm::vec4(ray_cast_origin, 1);
    glm::vec3 ray_cast_origin_vec3_local = glm::vec3(ray_cast_origin_vec4_local);
    std::cout<< "raycast origin LOCAL " << ray_cast_origin_vec3_local.x <<"."<<ray_cast_origin_vec3_local.y <<"."<<ray_cast_origin_vec3_local.z <<".\n";

    
    glm::vec4 ray_cast_direction_vec4_local = global_inverse * glm::vec4(ray_cast_direction_normalized, 0);
    glm::vec3 ray_cast_direction_vec3_local = glm::vec3(ray_cast_direction_vec4_local);

    for (unsigned int a = 0; a < vertex_arrays_of_geometry->size(); a++)
    {
        struct_vertex_array* vertex_array = vertex_arrays_of_geometry->at(a);
        for (unsigned int i = 0; i < vertex_array->indices->size(); i += 3)
        {
            vertex v0 = vertex_array->vertices->at(vertex_array->indices->at(i));
            vertex v1 = vertex_array->vertices->at(vertex_array->indices->at(i + 1));
            vertex v2 = vertex_array->vertices->at(vertex_array->indices->at(i + 2));


            glm::vec3 face_normal = glm::normalize(v0.normal + v1.normal + v2.normal);

            float d = -glm::dot(face_normal, v0.position);
            float t =
                -(glm::dot(face_normal, ray_cast_origin_vec3_local + d) /
                    glm::dot(face_normal, ray_cast_direction_vec3_local));

            glm::vec3 hit_point = ray_cast_origin_vec3_local + t * ray_cast_direction_vec3_local;
            auto c = glm::vec3();

            // Edge 0
            glm::vec3 edge0 = v1.position - v0.position;
            glm::vec3 vp0 = hit_point - v0.position;
            c = glm::cross(edge0, vp0);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            // Edge 1
            glm::vec3 edge1 = v2.position - v1.position;
            glm::vec3 vp1 = hit_point - v1.position;
            c = glm::cross(edge1, vp1);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            // Edge 2
            glm::vec3 edge2 = v0.position - v2.position;
            glm::vec3 vp2 = hit_point - v2.position;
            c = glm::cross(edge2, vp2);
            if (glm::dot(face_normal, c) < 0) continue; // P is on the right side

            // This ray hits the triangle
            if (ray_cast_hit->distance_from_origin > t)
            {
                ray_cast_hit->distance_from_origin = t;
                ray_cast_hit->hit = true;
                ray_cast_hit->distance_from_origin = t;
                ray_cast_hit->hit_world_space = hit_point;
                ray_cast_hit->hit_normal = face_normal;
            }
        }
    }
}
