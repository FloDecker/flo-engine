//
// Created by flode on 04/03/2023.
//

#include "Camera3D.h"

Camera3D::Camera3D(Object3D *parent, RenderContext *renderContext) : Object3D(parent) {
    this->renderContext_ = renderContext;
    forwardVectorLocal = glm::vec3 (0,0,-1);
}

void Camera3D::setRenderContext(RenderContext *renderContext) {
    this->renderContext_ = renderContext;
}

RenderContext *Camera3D::getRenderContext() {
    return this->renderContext_;
}

void Camera3D::calculateView() {
    this->drawSelf();
    this->renderContext_->camera->calculate_view_from_transform(this->transformGlobal, this->getWorldPosition(), this->getForwardVector());
}

camera* Camera3D::get_camera() const
{
    return this->renderContext_->camera;
}

int Camera3D::drawSelf() {
    return 0;
}


