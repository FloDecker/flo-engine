//
// Created by flode on 28/02/2023.
//
#pragma once
#include <glm.hpp>

#include <string>
#include "vector"
#include "../Editor/GlobalContext.h"
#include "../Renderer/RenderContext.h"

#ifndef ENGINE_OBJECT3D_H
#define ENGINE_OBJECT3D_H


const glm::vec3 vecX = glm::vec3 (1,0,0);
const glm::vec3 vecY = glm::vec3 (0,1,0);
const glm::vec3 vecZ = glm::vec3 (0,0,1);

//an object that can be in the 3D scene

class Object3D {
private:
    
    char *name;
    std::vector<unsigned int> tags_;
    
    glm::vec3 rotation_;
    glm::vec3 position_;
    glm::vec3 scale_;
    
    std::vector<Object3D*> children;
    Object3D *parent = nullptr;
    GlobalContext *global_context_;

    int draw_(struct RenderContext *parentRenderContext);
    void recalculateTransform(); //should be called after changing location / scale / rotation
    void recalculateTransformGlobal();

public:
    Object3D(GlobalContext *global_context);
    void add_tag(std::string tag);
    
    void setPositionLocal(glm::vec3 pos);
    void setPositionLocal(float x, float y, float z);

    void setRotationLocal(glm::vec3 rotation);
    void setRotationLocal(float x, float y, float z);

    void setScale(float x, float y, float z);
    void setScale(glm::vec3 scale);

    glm::vec3 getWorldPosition();
    glm::vec3 getLocalRotation();

    glm::vec3 getForwardVector();
    glm::vec3 getUpVector(); //TODO
    glm::vec3 getRightVector(); //TODO

    void addChild(Object3D *child);
    void drawEntryPoint(struct RenderContext *renderContext);
    std::vector<Object3D*> &get_children();
protected:
    glm::mat4 transformLocal = glm::mat4(1.0f); //local transform
    glm::mat4 transformGlobal = glm::mat4(1.0f); //global transform is recalculated for each frame //TODO: may optimize
    struct RenderContext *renderContext;
    glm::vec3 forwardVectorLocal = glm::vec3 (0,0,1);
    glm::vec3 upwardVectorLocal = glm::vec3 (0,1,0);
    glm::vec3 rightVectorLocal = glm::vec3 (1,0,0);

    virtual int drawSelf();

};



#endif //ENGINE_OBJECT3D_H
