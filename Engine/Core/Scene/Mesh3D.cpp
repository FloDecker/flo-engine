//
// Created by flode on 28/02/2023.
//

#include "Mesh3D.h"
#include "gtx/string_cast.hpp"


Mesh3D::Mesh3D(Mesh *mesh) {
    this->mesh = mesh;
}

int Mesh3D::drawSelf() {
    for (int i = 0; i < mesh->vertexArrays.size(); ++i) {
        //TODO: segfaults when there is no material for that vertex array
        ShaderProgram *p = mesh->materials[i]->shaderProgram;

        //TODO: projection doesen't have to be set at runtime -> only on projection changes
        // set model view projection
        p->use();
        p->setUniformMatrix4("mMatrix",glm::value_ptr(this->transformGlobal));
        p->setUniformMatrix4("vMatrix",glm::value_ptr(*this->renderContext->camera.getView()));
        p->setUniformMatrix4("pMatrix",glm::value_ptr(*this->renderContext->camera.getProjection()));
        mesh->vertexArrays[i]->draw();
    }
    return 1;
}

