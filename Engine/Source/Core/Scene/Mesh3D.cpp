//
// Created by flode on 28/02/2023.
//

#include "Mesh3D.h"


Mesh3D::Mesh3D(Mesh* mesh, GlobalContext* global_context) : Object3D(global_context)
{
    this->mesh = mesh;
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
            p = global_context_->default_shader;
        }

        //TODO: projection doesen't have to be set at runtime -> only on projection changes
        // set model view projection
        p->use();
        p->setUniformMatrix4("mMatrix", glm::value_ptr(this->transformGlobal));
        p->setUniformMatrix4("vMatrix", glm::value_ptr(*this->renderContext->camera.getView()));
        p->setUniformMatrix4("pMatrix", glm::value_ptr(*this->renderContext->camera.getProjection()));
        p->setUniformVec3F("cameraPosWS", glm::value_ptr(*this->renderContext->camera.getWorldPosition()));
        mesh->vertexArrays[i]->draw();
    }
    return 1;
}
