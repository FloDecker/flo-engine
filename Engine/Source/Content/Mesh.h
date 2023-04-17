//
// Created by flode on 14/03/2023.
//

#ifndef ENGINE_MESH_H
#define ENGINE_MESH_H

#include <vector>
#include "../Core/Renderer/VertexArray.h"
#include "Material.h"

class Mesh {
public:
    Mesh(){
        vertexArrays = {};
        materials = {};
    };
    std::vector<VertexArray *> vertexArrays;
    std::vector<ShaderProgram *> materials;
    void initializeVertexArrays();
};

inline void Mesh::initializeVertexArrays()
{
    for (auto element : vertexArrays)
    {
        element->load();
    }
}
#endif //ENGINE_MESH_H
