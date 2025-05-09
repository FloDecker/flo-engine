#include "primitive_instance.h"


#include "../Scene/Object3D.h"
#include "Primitives/primitive.h"
#include "Shader/ShaderProgram.h"
#include "../Scene/Lighting/light.h"
#include "../Scene/Scene.h"


primitive_instance::primitive_instance(Object3D* parent, ::primitive* primitive)
{
	this->primitive = primitive;
	this->parent_ = parent;
	this->shaderProgram_ = nullptr;
}

primitive_instance::primitive_instance(Object3D* parent, ::primitive* primitive,
                                       ShaderProgram* shaderProgram): primitive_instance(parent, primitive)
{
	set_shader(shaderProgram);
}

void primitive_instance::draw(RenderContext* render_context) const
{
	ShaderProgram* p;
	if (shaderProgram_ == nullptr || !shaderProgram_->is_compiled())
	{
		p = render_context->default_shader;
	}
	else
	{
		p = shaderProgram_;
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
	render_context->light_pass_depth_only_shader->
	                setUniformMatrix4("mMatrix", value_ptr(parent_->getGlobalTransform()));
	render_context->light_pass_depth_only_shader->setUniformMatrix4("lightSpaceMatrix",
	                                                                value_ptr(
		                                                                render_context->light->
		                                                                get_light_space_matrix()));
	primitive->draw();
}

void primitive_instance::draw_custom_pass(RenderContext* render_context, ShaderProgram* shader_program) const
{
	shader_program->use();
	shader_program->add_header_uniforms(parent_, render_context);
	primitive->draw();
}

bool primitive_instance::has_shader() const
{
	return shaderProgram_ != nullptr;
}

void primitive_instance::set_shader(ShaderProgram* shader_program)
{
	if (shader_program == nullptr)
	{
		shaderProgram_ = nullptr;
		return;
	}

	shaderProgram_ = shader_program;

	//TODO: not correct if there are multiple light maps 
	if (shaderProgram_->receives_dynamic_directional_light() && parent_->get_scene()->get_scene_direct_light() !=
		nullptr)
	{
		shaderProgram_->addTexture(parent_->get_scene()->get_scene_direct_light()->light_map(),
		                           "direct_light_map_texture");
	}
}
