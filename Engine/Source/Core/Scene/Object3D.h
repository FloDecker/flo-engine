#pragma once
#include <glm.hpp>

#include <string>

#include "vector"
#include "../Editor/GlobalContext.h"
#include "../Renderer/RenderContext.h"

const glm::vec3 vecX = glm::vec3(1, 0, 0);
const glm::vec3 vecY = glm::vec3(0, 1, 0);
const glm::vec3 vecZ = glm::vec3(0, 0, 1);

class Collider;

//an object that can be in the 3D scene

class Object3D
{
private:
    std::vector<unsigned int> tags_;

    glm::vec3 rotation_;
    glm::vec3 position_;
    glm::vec3 scale_ = {1.0,1.0,1.0};

    std::vector<Object3D*> children;
    Object3D* parent = nullptr;

    int draw_(struct RenderContext* parentRenderContext);
    void recalculateTransform(); //should be called after changing location / scale / rotation
    void recalculateTransformGlobal();

public:
    Object3D(GlobalContext* global_context);
    bool visible = true;
    std::string name;

    
    //TAGS
    void add_tag(std::string tag);
    bool has_tag(unsigned int tag_id) const;
    bool has_tag(const std::string& tag) const;

    // transform local //
    void setPositionLocal(glm::vec3 pos);
    void setPositionLocal(float x, float y, float z);

    void setRotationLocal(glm::vec3 rotation);
    void setRotationLocal(float x, float y, float z);

    void setRotationLocalDegrees(glm::vec3 rotation);
    void setRotationLocalDegrees(float x, float y, float z);

    void setScale(float scale);
    void setScale(float x, float y, float z);
    void setScale(glm::vec3 scale);

    //transform global
    void set_position_global(const glm::vec3& pos);
    void set_position_global(float x, float y, float z);
    void recalculate_global_transform();

    
    glm::mat4 getGlobalTransform();
    glm::vec3 getWorldPosition();
    glm::vec3 getLocalRotation();

    glm::vec3 getForwardVector();
    glm::vec3 getUpVector(); //TODO
    glm::vec3 getRightVector(); //TODO

    glm::vec3 get_scale();

    ////////////
    void drawEntryPoint(struct RenderContext* renderContext);
    void addChild(Object3D* child);
    Object3D* get_parent() const;
    std::vector<Object3D*>& get_children();
    Object3D* get_child_by_tag(std::string* tag);
protected:
    GlobalContext* global_context_;



    glm::mat4 transformLocal = glm::mat4(1.0f); //local transform
    glm::mat4 transformGlobal = glm::mat4(1.0f); //global transform is recalculated for each frame //TODO: may optimize
    struct RenderContext* renderContext;
    glm::vec3 forwardVectorLocal = glm::vec3(0, 0, 1);
    glm::vec3 upwardVectorLocal = glm::vec3(0, 1, 0);
    glm::vec3 rightVectorLocal = glm::vec3(1, 0, 0);

    virtual int drawSelf();
};
