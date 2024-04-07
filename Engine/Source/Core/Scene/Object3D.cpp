#include <iostream>
#include "Object3D.h"
#include "gtx/string_cast.hpp"

Object3D::Object3D(GlobalContext* global_context)
{
    global_context_ = global_context;
}

int Object3D::draw_(struct RenderContext *parentRenderContext) {

    //set global  uniforms
    renderContext = parentRenderContext;
    this->drawSelf();
    for (auto & child : children) {
        child->draw_(parentRenderContext);
    }
    return 0;
}

void Object3D::addChild(Object3D *child) {
    if (child->parent) {
        std::cerr << "Child already has parent" << std::endl;
        //TODO: allow reparenting
        return;
    }
    this->children.push_back(child);
    child->parent = this;
}

void Object3D::drawEntryPoint(RenderContext *renderContext) {
    draw_(renderContext);
}

int Object3D::drawSelf() {
    return 0;
}

//getter

glm::vec3 Object3D::getWorldPosition() {
    glm::vec4 temp = transformGlobal * glm::vec4 (0,0,0,1);
    return {temp.x,temp.y,temp.z};
}

glm::vec3 Object3D::getLocalRotation() {
    return rotation_;
}

glm::vec3 Object3D::getForwardVector() {
    glm::vec4 temp = transformGlobal * glm::vec4 (forwardVectorLocal,0);
    return glm::normalize(glm::vec3 (temp.x,temp.y,temp.z));
}

glm::vec3 Object3D::getRightVector() {
    glm::vec4 temp = transformGlobal * glm::vec4 (rightVectorLocal,0);
    return glm::normalize(glm::vec3 (temp.x,temp.y,temp.z));
}

std::vector<Object3D*> &Object3D::get_children()
{
    return children;
}

//setter for transform


void Object3D::setPositionLocal(float x, float y, float z) {
    setPositionLocal(glm::vec3(x, y, z));
}

void Object3D::setPositionLocal(glm::vec3 pos) {
    position_ = pos;
    recalculateTransform();
}

void Object3D::setRotationLocal(float x, float y, float z) {
    setRotationLocal(glm::vec3(x, y, z));
}

void Object3D::setRotationLocal(glm::vec3 rotation) {
    rotation_ = rotation;
    recalculateTransform();
}

void Object3D::setScale(float x, float y, float z) {
    setScale(glm::vec3(x,y,z));
}

void Object3D::setScale(glm::vec3 scale) {
    scale_ = scale;
    recalculateTransform();
}

void Object3D::recalculateTransform() {
    //TODO: add scale
    transformLocal = glm::mat4 (1.0f);

    //apply transform
    transformLocal = glm::translate(transformLocal, position_);

    //apply rotation
    transformLocal  = glm::rotate(transformLocal,rotation_.y,vecY);
    transformLocal  = glm::rotate(transformLocal,rotation_.x,vecX);
    transformLocal  = glm::rotate(transformLocal,rotation_.z,vecZ);


    this->recalculateTransformGlobal();
}

void Object3D::recalculateTransformGlobal() {
    this->transformGlobal = (parent)? parent->transformGlobal * this->transformLocal : this->transformLocal;
    for (auto child : this->children) {
        child->recalculateTransformGlobal();
    }
}

void Object3D::add_tag(std::string tag)
{
    auto tag_id = global_context_->tag_manager.get_id_of_tag(tag);
    if (tag_id < 0)
    {
        std::cout << "tag: " << tag.c_str() << " cant be added since its not part of the Tagmanager";
        return;
    }

    tags_.push_back(tag_id);
}
