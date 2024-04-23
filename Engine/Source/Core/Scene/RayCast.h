#pragma once
#include "SceneContext.h"
#include "../CommonDataStructures/StructVertexArray.h"

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

class RayCast
{
public:
    //ray cast in scene
    static RayCastHit ray_cast(
        SceneContext* scene_context,
        std::string *collision_tag,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction,
        float length,
        bool ignore_back_face = true
        );

    //TODO this shouldn't be used by the game developer !Engine only!
    static RayCastHit ray_cast_editor(
        SceneContext* scene_context,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction); 

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
    
};
