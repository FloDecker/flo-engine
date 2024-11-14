#pragma once
#include "Object3D.h"
#include "../Renderer/VertexArray.h"
#include "../../Content/Mesh.h"
#include "../Renderer/Shader/ShaderProgram.h"
#include <gtc/type_ptr.hpp>

//represents a 3D mesh

class Mesh3D : public Object3D
{
private :
    Mesh* mesh;

protected:
    int drawSelf() override;

public:
    explicit Mesh3D(Object3D* parent, Mesh* mesh);
    bool receive_light = true;
    Mesh* get_mesh() const;
    std::vector<ShaderProgram *> materials;
};
