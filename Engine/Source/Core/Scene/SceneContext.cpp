#include "SceneContext.h"



SceneContext::SceneContext(GlobalContext* global_context, Object3D* scene_root)
{
    global_context_ = global_context;
    scene_root_ = scene_root;

    //get ids of engine defined tags
    engine_light_point_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_LIGHT_POINT");
    engine_collider_id_ = global_context_->tag_manager.get_id_of_tag("ENGINE_COLLIDER");
    scene_bb = new StackedBB(&sceneColliders);
    recalculate_from_root();
}


void SceneContext::recalculate_at(Object3D* parent)
{
    //sort objets by their tag
    if (parent->has_tag(engine_light_point_id_))
    {
        this->scenePointLights.insert(dynamic_cast<PointLight*>(parent));
    }

    if (parent->has_tag(engine_collider_id_))
    {
        this->sceneColliders.push_back(dynamic_cast<Collider*>(parent));
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
    calculateColliderBoundingBoxes();
    scene_bb->recalculate();
}


void SceneContext::calculateColliderBoundingBoxes()
{
    for (auto collider : sceneColliders)
    {
        collider->calculate_world_space_bounding_box();

        //TODO REMOVE THIS TEST
        //auto c = new Cube3D(global_context_);
        //scene_root_->addChild(c);
        //c->setScale(BoundingBoxHelper::get_scale_of_bb(&collider->bounding_box));
        //c->color = {0, 0, 1};
        //c->set_position_global(BoundingBoxHelper::get_center_of_bb(&collider->bounding_box));
        ////////////////////TEST END //////////////////////////
    }
}

Object3D* SceneContext::get_root() const
{
    return scene_root_;
}

GlobalContext* SceneContext::get_global_context() const
{
    return global_context_;
}

StackedBB* SceneContext::get_bb() const
{
    return scene_bb;
}

void SceneContext::recalculate_scene_bb()
{
    
}
