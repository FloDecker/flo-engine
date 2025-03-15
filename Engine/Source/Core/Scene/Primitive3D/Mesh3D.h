#pragma once
#include "../Object3D.h"
#include "../../Renderer/Primitives/vertex_array.h"
#include "../../Content/Mesh.h"
#include "../../Renderer/Shader/ShaderProgram.h"
#include <gtc/type_ptr.hpp>

#include "i_set_material.h"
#include "../../Renderer/primitive_instance.h"

//represents a 3D mesh

class Mesh3D : public Object3D, public i_set_material
{
   

private:
    Mesh* mesh_;
    std::vector<primitive_instance *> primitives_;

protected:
    int drawSelf() override;
    int draw_self_shadow_pass() override;
    int draw_self_custom_pass(ShaderProgram* shader_program) override;

public:
    explicit Mesh3D(Object3D* parent, Mesh* mesh);
    bool set_material(ShaderProgram* shader_program) override;
    bool set_material_at(ShaderProgram *material, unsigned int pos) const;
};
