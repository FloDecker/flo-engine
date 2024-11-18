#pragma once
#include <glm.hpp>

#include <string>

#include "imgui.h"
#include "vector"
#include "../Editor/GlobalContext.h"
#include "../Renderer/RenderContext.h"

//Forward declaration
class Scene;

const glm::vec3 vecX = glm::vec3(1, 0, 0);
const glm::vec3 vecY = glm::vec3(0, 1, 0);
const glm::vec3 vecZ = glm::vec3(0, 0, 1);


//an object is the most basic type in a Scene

class Object3D
{

    friend class SceneRoot;
    
private:
    Object3D(); //friend class SceneRoot overwrites this since it's the only Object3D without parent
    std::vector<unsigned int> tags_; //tags associated with this object

    //transforms
    glm::vec3 rotation_ = {0.0,0.0,0.0};
    glm::vec3 position_ = {0.0,0.0,0.0};
    glm::vec3 scale_ = {1.0,1.0,1.0};

    //hierarchy
    std::vector<Object3D*> children;
    Object3D* parent = nullptr;

    
    glm::mat4 global_transform_inverse_;

    
    int draw_(struct RenderContext* parentRenderContext);
    void recalculateTransform(); //should be called after changing location / scale / rotation
    void recalculateTransformGlobal();

    

public:
    Object3D(Object3D* parent);
    bool visible = true;
    std::string name;

    
    //TAGS
    void add_tag(std::string tag);
    void remove_tag(std::string tag);
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


    //Transform Helper

    //takes a vertex defined in the objects local space and returns the world space coordinates of it
    glm::vec3 transform_vertex_to_world_space(const glm::vec3& vertex_in_local_space) const;
    
    //Getter
    glm::mat4 getGlobalTransform();
    glm::mat4 getGlobalTransformInverse();
    glm::vec3 getWorldPosition();
    glm::vec3 getLocalRotation();
    glm::vec3 getLocalRotationDegrees();

    glm::vec3 getForwardVector();
    glm::vec3 getUpVector(); //TODO
    glm::vec3 getRightVector(); //TODO

    glm::vec3 get_scale();

    ////////////
    void addChild(Object3D* child);
    Object3D* get_parent() const;
    std::vector<Object3D*>& get_children();
    Object3D* get_child_by_tag(std::string* tag);

    //UI
    void ui_get_scene_structure_recursively(ImGuiTreeNodeFlags flags) const;
protected:
    
    Scene* scene_;
    glm::mat4 transformLocal = glm::mat4(1.0f); //local transform
    glm::mat4 transformGlobal = glm::mat4(1.0f); //global transform is recalculated for each frame //TODO: may optimize
    struct RenderContext* renderContext;
    GlobalContext *global_context_;
    glm::vec3 forwardVectorLocal = glm::vec3(0, 0, 1);
    glm::vec3 upwardVectorLocal = glm::vec3(0, 1, 0);
    glm::vec3 rightVectorLocal = glm::vec3(1, 0, 0);
    virtual int drawSelf();

    //FLAGS
    bool IGNORE_IN_SCENE_TREE_VIEW = false;
};
