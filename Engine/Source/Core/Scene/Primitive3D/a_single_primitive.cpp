#include "a_single_primitive.h"

bool a_single_primitive::set_material(ShaderProgram* shader_program)
{
	primitive_->set_shader(shader_program);
	return true;
}

int a_single_primitive::drawSelf()
{
	primitive_->draw(this->renderContext);
	return 1;
}

int a_single_primitive::draw_self_shadow_pass()
{
	primitive_->draw_shadow_pass(this->renderContext);
	return 1;
}

int a_single_primitive::draw_self_custom_pass(ShaderProgram* shader_program)
{
	primitive_->draw_custom_pass(this->renderContext, shader_program);
	return 1;
}
