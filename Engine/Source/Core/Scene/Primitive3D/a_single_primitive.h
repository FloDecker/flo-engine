#pragma once
#include "i_set_material.h"
#include "../Object3D.h"
#include "../../Renderer/primitive_instance.h"

class a_single_primitive : public Object3D, public i_set_material
{
public:
	bool set_material(ShaderProgram* shader_program) override;

	explicit a_single_primitive(Object3D* parent, primitive_instance* primitive): Object3D(parent)
	{
		primitive_ = primitive;
	}

protected:
	int drawSelf() override;
	int draw_self_shadow_pass() override;
	int draw_self_custom_pass(ShaderProgram* shader_program) override;

private:
	primitive_instance* primitive_;
};
