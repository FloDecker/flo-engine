//
// Created by flode on 28/02/2023.
//

#include "Mesh.h"
#include "gtx/string_cast.hpp"

int Mesh::drawSelf() {
    vertexArray_->model = &this->transformGlobal;
    vertexArray_->projection = this->renderContext->camera.getProjection();
    vertexArray_->view = this->renderContext->camera.getView();
    vertexArray_->draw();
}

void Mesh::setVertexArray(VertexArray *vertexArray) {
    this->vertexArray_ = vertexArray;
    vertexArray->load();
}
