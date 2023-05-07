//
// Created by flode on 02/03/2023.
//
#include "RenderContext.h"
#include "gtc/matrix_transform.hpp"
Camera::Camera(float width, float height) {
    view = glm::mat4 (1.0);
    setViewPortDimension(width,height);
}

void Camera::setViewPortDimension(float width, float height) {
    this->width_ = width;
    this->height_ = height;
    recalculateProjection();
}

void Camera::recalculateProjection() {
    this->projection = glm::perspective(glm::radians(FOV_),this->width_ / this->height_, nearClippingPlane_,farClippingPlane_);
}

void Camera::setClippingPlanes(float near, float far) {
    nearClippingPlane_ = near;
    farClippingPlane_ = far;
    recalculateProjection();
}

void Camera::setFOV(float FOV) {
    FOV_ = FOV;
    recalculateProjection();
}

glm::mat4 *Camera::getProjection() {
    return &projection;
}

glm::mat4 *Camera::getView() {
    return &view;
}

glm::vec3 *Camera::getWorldPosition()
{
    return &positionWS_;
}



void Camera::calculateView(glm::mat4 cameraTransform, glm::vec3 cameraPos, glm::vec3 cameraViewDirection) {
    view = glm::inverse(cameraTransform);
    positionWS_ = cameraPos;
    viewDirection_ = cameraViewDirection;
}


