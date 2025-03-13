#pragma once

struct RenderContext;
class primitive;
class ShaderProgram;
class Object3D;

class primitive_instance
{
public:
	primitive_instance(Object3D* parent, ShaderProgram* shaderProgram, primitive * primitive);
	ShaderProgram* shaderProgram;
	primitive * primitive;

	void draw(RenderContext* render_context) const;
	void draw_shadow_pass(RenderContext* render_context) const;
	void draw_custom_pass(RenderContext* render_context, ShaderProgram* shader_program) const;
	
private:
	Object3D* parent_;
};
