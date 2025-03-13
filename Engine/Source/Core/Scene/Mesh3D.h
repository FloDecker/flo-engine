#pragma once
#include "Object3D.h"
#include "../Renderer/Primitives/vertex_array.h"
#include "../../Content/Mesh.h"
#include "../Renderer/Shader/ShaderProgram.h"
#include <gtc/type_ptr.hpp>

//represents a 3D mesh

class Mesh3D : public Object3D
{
private :
    Mesh* mesh;
    std::vector<ShaderProgram *> materials;

protected:
    int drawSelf() override;
    int draw_self_shadow_pass() override;
    int draw_self_custom_pass(ShaderProgram* shader_program) override;

public:
    explicit Mesh3D(Object3D* parent, Mesh* mesh);
    bool receive_light = true;

    bool add_material(ShaderProgram *material);
};
