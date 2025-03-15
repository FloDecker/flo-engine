#pragma once

class ShaderProgram;

class i_set_material
{
public:
	virtual ~i_set_material() = default;
	virtual bool set_material(ShaderProgram* shader_program) = 0;
};
