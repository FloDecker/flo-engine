#pragma once
#include <unordered_set>

#include "Object3D.h"
#include "../Editor/GlobalContext.h"
#include "Lighting/PointLight.h"
#include "../../Util/StackedBB.h"

//scene context is used to feed scene information to the material shaders 
class SceneContext
{
public:
    SceneContext(GlobalContext* global_context, Object3D* scene_root);
    std::vector<PointLight*> get_scene_point_lights();
    void recalculate_at(Object3D* parent);
    void recalculate_from_root();
    void calculateColliderBoundingBoxes();
    Object3D* get_root() const;
    GlobalContext* get_global_context() const;
    StackedBB* get_bb() const;
    
private:
    std::unordered_set<PointLight*> scenePointLights;
    std::vector<Collider*> sceneColliders;
    StackedBB *scene_bb;

    void recalculate_scene_bb();

    GlobalContext* global_context_;
    Object3D* scene_root_;

    //ids for engine defined tags
    unsigned int engine_light_point_id_;
    unsigned int engine_collider_id_;
};
