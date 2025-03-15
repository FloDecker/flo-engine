#include "Line3D.h"

#include <gtc/type_ptr.hpp>
#include <gtx/dual_quaternion.hpp>

Line3D::Line3D(Object3D* parent, glm::vec3 pos_0, glm::vec3 pos_1): Object3D(
	parent)
{
	this->pos_0 = pos_0;
	this->pos_1 = pos_1;
	//TODO add directly to root
	//scene_root->addChild(this);
	set_positions(pos_0, pos_1);
}

int Line3D::drawSelf()
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
	global_context_->global_primitives.line->draw();
	return 1;
}

void Line3D::set_positions(glm::vec3 pos_0, glm::vec3 pos_1)
{
	this->pos_0 = pos_0;
	this->pos_1 = pos_1;
	float distance = glm::distance(pos_0, pos_1);
	this->setScale(distance, distance, distance);
	this->set_position_global(pos_0);
	auto direction_normalized = normalize(pos_0 - pos_1);

	setRotationLocal(quatLookAt(direction_normalized, vec_y));
}
