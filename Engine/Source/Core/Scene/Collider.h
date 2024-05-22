#pragma once
#include "Mesh3D.h"
#include "Object3D.h"
#include "RayCast.h"
#include "../CommonDataStructures/StructBoundingBox.h"
#include "../../Util/BoundingBoxHelper.h"

class Collider : public Object3D
{
public:
    Collider(GlobalContext* global_context);
    virtual void check_collision(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length,
                                 bool ignore_back_face, RayCastHit* ray_cast_hit);
    virtual bool is_in_proximity(glm::vec3 center_ws, float radius);
};

class MeshCollider : public Collider
{
public:
    MeshCollider(GlobalContext* global_context, const std::vector<struct_vertex_array*>& vertex_arrays);
    MeshCollider(GlobalContext* global_context, Mesh3D* mesh);
    std::vector<struct_vertex_array*>* get_vertex_arrays();
    void check_collision(glm::vec3 ray_origin_ws, glm::vec3 ray_direction_ws, float ray_length, bool ignore_back_face,
                         RayCastHit* ray_cast_hit) override;
    virtual bool is_in_proximity(glm::vec3 center_ws, float radius);
    StructBoundingBox bounding_box;
private:
    std::vector<struct_vertex_array*> vertex_arrays_;
};
