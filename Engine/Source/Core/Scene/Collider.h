#pragma once
#include "Mesh3D.h"
#include "Object3D.h"
#include "../CommonDataStructures/StructVertexArray.h"

class Collider : public Object3D
{
public:
    Collider(GlobalContext* global_context);
};

class MeshCollider : public Collider
{
public:
    MeshCollider(GlobalContext* global_context, const std::vector<struct_vertex_array*>& vertex_arrays);
    MeshCollider(GlobalContext* global_context, Mesh3D* mesh);
    std::vector<struct_vertex_array*>* get_vertex_arrays();

private:
    std::vector<struct_vertex_array*> vertex_arrays_;
};
