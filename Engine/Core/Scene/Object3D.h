//
// Created by flode on 28/02/2023.
//
#include "vector"
#include "../Renderer/RenderContext.h"
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>

#ifndef ENGINE_OBJECT3D_H
#define ENGINE_OBJECT3D_H


const glm::vec3 vecX = glm::vec3 (1,0,0);
const glm::vec3 vecY = glm::vec3 (0,1,0);
const glm::vec3 vecZ = glm::vec3 (0,0,1);

class Object3D {
private:
    char *name;

    glm::vec3 rotation_;
    glm::vec3 position_;
    glm::vec3 scale_;

    std::vector<Object3D*> children;
    Object3D *parent = nullptr;

    int draw_(struct RenderContext *parentRenderContext);
    void recalculateTransform(); //should be called after changing location / scale / rotation
    void recalculateTransformGlobal();

public:
    void setPositionLocal(glm::vec3 pos);
    void setPositionLocal(float x, float y, float z);

    void setRotationLocal(glm::vec3 rotation);
    void setRotationLocal(float x, float y, float z);

    void setScale(float x, float y, float z);
    void setScale(glm::vec3 scale);

    glm::vec3 getWorldPosition();

    glm::vec3 getForwardVector();
    glm::vec3 getUpVector(); //TODO
    glm::vec3 getRightVector(); //TODO

    void addChild(Object3D *child);
    void drawEntryPoint(struct RenderContext *renderContext);


protected:
    glm::mat4 transformLocal = glm::mat4(1.0f); //local transform
    glm::mat4 transformGlobal = glm::mat4(1.0f); //global transform is recalculated for each frame //TODO: may optimize
    struct RenderContext *renderContext;

    virtual int drawSelf();

};



#endif //ENGINE_OBJECT3D_H
