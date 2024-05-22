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
    if (!visible) return 0;
    this->drawSelf();
    for (auto & child : children) {
        child->draw_(parentRenderContext);
    }
    return 1;
}

void Object3D::addChild(Object3D *child) {
    if (child->parent) {
        std::cerr << "Child already has parent" << std::endl;
        //TODO: allow reparenting
        return;
    }

    //TODO : maybe there is a more elegant solution for this
    auto pos_global = child->getWorldPosition();
    this->children.push_back(child);
    child->parent = this;
    child->set_position_global(pos_global);
}

Object3D* Object3D::get_parent() const
{
    return parent;
}

void Object3D::drawEntryPoint(RenderContext *renderContext) {
    draw_(renderContext);
}

int Object3D::drawSelf() {
    return 0;
}

//getter

glm::mat4 Object3D::getGlobalTransform()
{
    return transformGlobal;
}


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

glm::vec3 Object3D::get_scale()
{
    return scale_;
}

std::vector<Object3D*> &Object3D::get_children()
{
    return children;
}

Object3D* Object3D::get_child_by_tag(std::string* tag)
{
    for (auto child : children)
    {
        if (child->has_tag(*tag)) return child;
    }
    return nullptr;
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

void Object3D::setRotationLocalDegrees(glm::vec3 rotation)
{
    setRotationLocal(glm::radians(rotation));
}

void Object3D::setRotationLocalDegrees(float x, float y, float z)
{
    setRotationLocalDegrees(glm::vec3(x,y,z));
}

void Object3D::setRotationLocal(glm::vec3 rotation) {
    rotation_ = rotation;
    recalculateTransform();
}

void Object3D::setScale(float scale)
{
    setScale(glm::vec3(scale));
}

void Object3D::setScale(float x, float y, float z) {
    setScale(glm::vec3(x,y,z));
}

void Object3D::setScale(glm::vec3 scale) {
    scale_ = scale;
    recalculateTransform();
}

void Object3D::set_position_global(const glm::vec3& pos)
{
    const auto transform_inverse = inverse(parent->getGlobalTransform());
    const auto pos_new = transform_inverse * glm::vec4(pos, 1);
    auto scale_inverse = new glm::mat4(1.0); 
    setPositionLocal(glm::vec3(pos_new * glm::scale(*scale_inverse, glm::vec3(1,1,1) / scale_)));
    free(scale_inverse);
}

void Object3D::set_position_global(float x, float y, float z)
{
    set_position_global(glm::vec3(x,y,z));
}

void Object3D::recalculate_global_transform()
{
    this->recalculateTransform();
}


void Object3D::recalculateTransform() {
    transformLocal = glm::mat4 (1.0f);

    //apply scale
    transformLocal = glm::scale(transformLocal, scale_);

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

//tags

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

bool Object3D::has_tag(const unsigned tag_id) const
{
    for (const auto tag : tags_)
    {
        if (tag == tag_id)
        {
            return true;
        }
    }
    return false;
}

bool Object3D::has_tag(const std::string& tag) const
{
    const auto id = global_context_->tag_manager.get_id_of_tag(tag);
    if (id < 0)
    {
        return false;
    }

    return has_tag(id);
}
