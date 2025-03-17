#include "Cube3D.h"

#include <gtc/type_ptr.hpp>

Cube3D::Cube3D(Object3D* parent) : Object3D(parent)
{
	cube_ = new Cube();
	cube_->load();
}

int Cube3D::drawSelf()
{
	global_context_->default_color_debug_shader->use();
	global_context_->default_color_debug_shader->setUniformMatrix4("mMatrix", value_ptr(this->transformGlobal));
	global_context_->default_color_debug_shader->setUniformMatrix4(
		"vMatrix", value_ptr(*this->renderContext->camera->getView()));
	global_context_->default_color_debug_shader->setUniformMatrix4(
		"pMatrix", value_ptr(*this->renderContext->camera->getProjection()));
	global_context_->default_color_debug_shader->set_uniform_vec3_f("cameraPosWS",
	                                                                value_ptr(
		                                                                *this->renderContext->camera->
		                                                                       getWorldPosition()));
	global_context_->default_color_debug_shader->set_uniform_vec3_f("color", value_ptr(color));
	cube_->draw();
	return 1;
}
