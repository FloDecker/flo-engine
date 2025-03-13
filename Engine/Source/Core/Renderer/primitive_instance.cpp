#include "primitive_instance.h"

#include <gtc/type_ptr.hpp>

#include "../Scene/Object3D.h"
#include "Primitives/primitive.h"
#include "Shader/ShaderProgram.h"
#include "../Scene/Lighting/light.h"



primitive_instance::primitive_instance(Object3D* parent, ShaderProgram* shaderProgram, ::primitive* primitive)
{
	this->shaderProgram = shaderProgram;
	this->primitive = primitive;
	this->parent_ = parent;
}

void primitive_instance::draw(RenderContext* render_context) const
{
	ShaderProgram *p;
	if (shaderProgram == nullptr || !shaderProgram->is_compiled())
	{
		p = render_context->default_shader;
	} else
	{
		p = shaderProgram;
	}

	//TODO: projection doesen't have to be set at runtime -> only on projection changes
	// set model view projection
	p->use();
	p->add_header_uniforms(parent_, render_context);
	primitive->draw();
}

void primitive_instance::draw_shadow_pass(RenderContext* render_context) const
{
	render_context->light_pass_depth_only_shader->use();
	render_context->light_pass_depth_only_shader->setUniformMatrix4("mMatrix", glm::value_ptr(parent_->getGlobalTransform()));
	render_context->light_pass_depth_only_shader->setUniformMatrix4("lightSpaceMatrix",
							  glm::value_ptr(render_context->light->get_light_space_matrix()));
	primitive->draw();
}

void primitive_instance::draw_custom_pass(RenderContext* render_context, ShaderProgram* shader_program) const
{
	shader_program->use();
	shader_program->add_header_uniforms(parent_, render_context);
	primitive->draw();
}
