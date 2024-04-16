#include "Collider.h"

Collider::Collider(GlobalContext* global_context): Object3D(global_context)
{
    add_tag("ENGINE_COLLIDER");
}



//Mesh Collider
MeshCollider::MeshCollider(GlobalContext* global_context, const std::vector<struct_vertex_array*>& vertex_arrays): Collider(global_context)
{
    vertex_arrays_ = vertex_arrays;
}

MeshCollider::MeshCollider(GlobalContext* global_context, Mesh3D* mesh): Collider(global_context)
{
    for (auto vertex_array : mesh->get_mesh()->vertexArrays)
    {
        vertex_arrays_.push_back(vertex_array->get_vertex_array());
    }
    mesh->addChild(this);
}

std::vector<struct_vertex_array*>* MeshCollider::get_vertex_arrays()
{
    return &vertex_arrays_;
}
