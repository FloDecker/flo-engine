#include "Mesh3D.h"

#include "../Scene.h"
#include "../Modifiers/Implementations/Colliders/mesh_collider.h"

Mesh3D::Mesh3D(Object3D* parent, Mesh* mesh) : Object3D(parent)
{
	this->mesh_ = mesh;
	for (int i = 0; i < this->mesh_->vertexArrays.size(); i++)
	{
		auto v = this->mesh_->vertexArrays[i];
		auto s = (this->mesh_->materials.size() > i) ? this->mesh_->materials[i] : nullptr;

		auto p = new primitive_instance(this, v,s);
		primitives_.push_back(p);
	}
	auto f = new mesh_collider(this, mesh);
	add_modifier(f);
}

bool Mesh3D::set_material(ShaderProgram* shader_program)
{
	return set_material_at(shader_program, 0);
}

bool Mesh3D::set_material_at(ShaderProgram* material, unsigned int pos) const
{
	if (material == nullptr)
	{
		return false;
	}
	if (pos >= primitives_.size())
	{
		return false;
	}
	
	primitives_.at(pos)->set_shader(material);
	return true;
}

int Mesh3D::drawSelf()
{
	for (auto p: primitives_)
	{
		p->draw(this->renderContext);
	}
	return 1;
}

int Mesh3D::draw_self_shadow_pass()
{
	for (auto p: primitives_)
	{
		p->draw_shadow_pass(this->renderContext);
	}
	return 1;
}

int Mesh3D::draw_self_custom_pass(ShaderProgram* shader_program)
{
	for (auto p: primitives_)
	{
		p->draw_custom_pass(this->renderContext, shader_program);
	}
	return 1;
}
