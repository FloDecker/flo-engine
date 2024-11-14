#pragma once
#include "Scene.h"
#include "../CommonDataStructures/StructVertexArray.h"
#include "../../Util/RayIntersectionHelper.h"

class RayCast
{
public:
    //ray cast in scene
    static RayCastHit ray_cast(
        Scene* scene_context,
        std::string* collision_tag,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction,
        float length,
        bool ignore_back_face = true
    );

    //TODO this shouldn't be used by the game developer !Engine only!
    static RayCastHit ray_cast_editor(
        Scene* scene_context,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction,
        bool ignore_back_face = true);



private:
    static void recurse_scene_model_ray_cast(
        RayCastHit* ray_cast_hit,
        std::string* collision_tag,
        Object3D* object,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction_normalized,
        double length,
        bool ignore_back_face = true
    );

    static bool recurse_proximity_check(
        Object3D* object,
        std::string* collision_tag,
        glm::vec3 proximity_center,
        float radius
    );


};
