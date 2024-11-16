#pragma once

#include <unordered_set>

#include "Handle.h"
#include "Object3D.h"
#include "../Editor/GlobalContext.h"
#include "Lighting/PointLight.h"
#include "../../Util/StackedBB.h"
#include "../CommonDataStructures/StructMeshTriangleFilter.h"

class SceneRoot: public Object3D
{
    public:
    SceneRoot(GlobalContext *global_context, Scene* scene);
    int draw_entry_point(RenderContext* render_context) const;
};

//scene context contains scene specific information
class Scene
{
public:
    Scene(GlobalContext* global_context , const std::string& name);
    std::vector<PointLight*> get_scene_point_lights();
    void recalculate_at(Object3D* parent);
    void recalculate_from_root();
    void calculateColliderBoundingBoxes();
    Object3D* get_root() const;
    GlobalContext* get_global_context() const;
    StackedBB* get_bb() const;
    void draw_scene(RenderContext *render_context) const;

    std::vector<Collider*>* get_colliders_in_bounding_box(StructBoundingBox *bounding_box) const;

    std::vector<glm::vec3>* get_polygons_in_bounding_box(StructBoundingBox* bounding_box) const;

    std::vector<std::tuple<MeshCollider*,std::vector<vertex_array_filter>*>>* get_triangles_in_bounding_box(StructBoundingBox* bounding_box) const;

private:
    std::unordered_set<PointLight*> scenePointLights;
    std::vector<Collider*> sceneColliders;
    StackedBB *scene_bb;
    GlobalContext* global_context_;

    SceneRoot* scene_root_; 
    
    //ids for engine defined tags
    unsigned int engine_light_point_id_;
    unsigned int engine_collider_id_;

    //name of the scene
    std::string name_;
};
