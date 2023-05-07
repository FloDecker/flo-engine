//
// Created by flode on 04/03/2023.
//

#include "Camera3D.h"

void Camera3D::setRenderContext(RenderContext *renderContext) {
    this->renderContext_ = renderContext;
}

RenderContext *Camera3D::getRenderContext() {
    return this->renderContext_;
}

void Camera3D::calculateView() {
    this->drawSelf();
    this->renderContext_->camera.calculateView(this->transformGlobal, this->getWorldPosition(), this->getForwardVector());
}

int Camera3D::drawSelf() {
    return 0;
}

Camera3D::Camera3D(RenderContext *renderContext) {
    this->renderContext_ = renderContext;
    forwardVectorLocal = glm::vec3 (0,0,-1);
}

