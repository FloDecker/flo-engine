#include "RayCast.h"

#include <chrono>
#include <gtx/string_cast.hpp>

#include "Collider.h"
#include "Mesh3D.h"


RayCastHit RayCast::ray_cast(SceneContext* scene_context, glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction,
                             float length,
                             bool ignore_back_face)
{
    auto start = std::chrono::system_clock::now();
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
    auto end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "raycast time total: " << elapsed_seconds.count() << "s\n";
    return ray_cast_hit;
}

void RayCast::recurse_scene_model_ray_cast(RayCastHit* ray_cast_hit, Object3D* object,
                                           glm::vec3 ray_cast_origin, glm::vec3 ray_cast_direction_normalized,
                                           double length, bool ignore_back_face)
{
    if (object->has_tag("ENGINE_COLLIDER") && object->visible)
    {
        auto mesh_collider = dynamic_cast<Collider*>(object);
        mesh_collider->check_collision(ray_cast_origin,ray_cast_direction_normalized,length,ignore_back_face,ray_cast_hit);
    }
    for (auto child : object->get_children())
    {
        recurse_scene_model_ray_cast(ray_cast_hit, child, ray_cast_origin, ray_cast_direction_normalized, length,
                                     ignore_back_face);
    }
}

