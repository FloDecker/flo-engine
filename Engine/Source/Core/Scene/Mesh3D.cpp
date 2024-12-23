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
		p->setUniformMatrix4("mMatrix", glm::value_ptr(this->transformGlobal));
		p->setUniformMatrix4("vMatrix", glm::value_ptr(*this->renderContext->camera->getView()));
		p->setUniformMatrix4("pMatrix", glm::value_ptr(*this->renderContext->camera->getProjection()));
		p->set_uniform_vec3_f("cameraPosWS", glm::value_ptr(*this->renderContext->camera->getWorldPosition()));
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
