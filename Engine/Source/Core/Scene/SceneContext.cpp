#include "SceneContext.h"


SceneContext::SceneContext(GlobalContext *global_context, Object3D *scene_root)
{
    global_context_ = global_context;
    scene_root_ = scene_root;
}


void SceneContext::recalculate_at(Object3D* parent)
{
    const auto children = parent->get_children();
    for (Object3D* child : children)
    {
        // extract data from every scene object here
        //TODO: can be optimized
        
        // if ()
        // {
        //     this->scenePointLights.insert(PointLight (*child));
        // }
        
        this->recalculate_at(child);
    }
}

void SceneContext::recalculate_from_root()
{
    recalculate_at(scene_root_);
}


