#pragma once
#include <unordered_set>

#include "Object3D.h"
#include "../Editor/GlobalContext.h"
#include "Lighting/PointLight.h"

class SceneContext
{
public:
    SceneContext(GlobalContext *global_context, Object3D *scene_root);
    std::vector<PointLight*> get_scene_point_lights();
    void recalculate_at(Object3D* parent);
    void recalculate_from_root();
    
private:
    std::unordered_set<PointLight*> scenePointLights;
    GlobalContext *global_context_;
    Object3D *scene_root_;

    
};
