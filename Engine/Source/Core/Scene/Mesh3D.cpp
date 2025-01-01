#include "Mesh3D.h"

#include "Collider.h"
#include "Scene.h"

Mesh3D::Mesh3D(Object3D* parent, Mesh* mesh) : Object3D(parent)
{
	this->mesh = mesh;
	auto m = new MeshCollider(this, this);
	m->add_tag("ENGINE_COLLIDER");

	
}

Mesh* Mesh3D::get_mesh() const
{
	return mesh;
}

bool Mesh3D::add_material(ShaderProgram* material)
{
	if (material == nullptr)
	{
		return false;
	}
	materials.push_back(material);
	if (material->receives_dynamic_directional_light() && scene_->get_scene_direct_light() != nullptr)
	{
		material->addTexture(scene_->get_scene_direct_light()->light_map(),"direct_light_map_texture");
	}
	return true;
}

int Mesh3D::drawSelf()
{
	for (int i = 0; i < mesh->vertexArrays.size(); ++i)
	{
		ShaderProgram* p = nullptr;
		if (i < this->materials.size()) //check first materials of this 3D object
		{
			p = this->materials[i];
		}
		else if (i < mesh->materials.size()) //check material of mesh
		{
			p = mesh->materials[i];
		}
		else //use default material
		{
			p = scene_->get_global_context()->default_shader;
		}

		if (!p->is_compiled())
		{
			p = scene_->get_global_context()->default_shader;
		}

		//TODO: projection doesen't have to be set at runtime -> only on projection changes
		// set model view projection
		p->use();
		p->add_header_uniforms(this, this->renderContext);
		mesh->vertexArrays[i]->draw();
	}
	return 1;
}

int Mesh3D::draw_self_shadow_pass()
{
	for (int i = 0; i < mesh->vertexArrays.size(); ++i)
	{
		auto shader = scene_->get_global_context()->light_pass_depth_only_shader;
		shader->use();
		shader->setUniformMatrix4("mMatrix", glm::value_ptr(this->transformGlobal));
		shader->setUniformMatrix4("lightSpaceMatrix",
		                          glm::value_ptr(this->renderContext->light->get_light_space_matrix()));
		mesh->vertexArrays[i]->draw();
	}
	return 1;
}
