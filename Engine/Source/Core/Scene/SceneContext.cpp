#include "SceneContext.h"

#include <iostream>


SceneContext::SceneContext(GlobalContext *global_context, Object3D *scene_root)
{
    global_context_ = global_context;
    scene_root_ = scene_root;

    //get ids of engine defined tags
    engine_light_point_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_LIGHT_POINT");
    engine_collider_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_COLLIDER");
    recalculate_from_root();
}


void SceneContext::recalculate_at(Object3D* parent)
{
    //sort objets by their tag
    if (parent->has_tag(engine_light_point_id_))
    {
        this->scenePointLights.insert(dynamic_cast<PointLight*> (parent));
    }
    
    const auto children = parent->get_children();
    for (Object3D* child : children)
    {
        this->recalculate_at(child);
    }
}

void SceneContext::recalculate_from_root()
{
    recalculate_at(scene_root_);
}

Object3D* SceneContext::get_root() const
{
    return scene_root_;
}


