#pragma once
#include "SceneContext.h"
#include "../CommonDataStructures/StructVertexArray.h"

struct RayCastHit
{
    bool hit; //if the raycast hit something
    double distance_from_origin;
    Object3D* object_3d; //the object that was hit
    glm::vec3 hit_world_space; //hit intersection in world space
    glm::vec3 hit_normal; //normal of the hit
};

class RayCast
{
public:
    //ray cast in scene
    static RayCastHit ray_cast(
        SceneContext* scene_context,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction,
        float length,
        bool ignore_back_face = true
        );

private:
    //ray cast within an object
    static void geometry_ray_cast(
        RayCastHit* ray_cast_hit,
        std::vector<struct_vertex_array*>* vertex_arrays_of_geometry,
        glm::mat4 global_transform,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction_normalized,
        double length,
        bool ignore_back_face = true
        );

    static void recurse_scene_model_ray_cast(
        RayCastHit* ray_cast_hit,
        Object3D* object,
        glm::vec3 ray_cast_origin,
        glm::vec3 ray_cast_direction_normalized,
        double length,
        bool ignore_back_face = true
        );
    
};
