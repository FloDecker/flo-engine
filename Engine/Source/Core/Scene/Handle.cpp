#include "Handle.h"

#include "Collider.h"
#include "Scene.h"
#include "../../Util/RayIntersectionHelper.h"


Handle::Handle(Scene *scene): Object3D(scene->get_root())
{
    //load handler models
    auto engine_handler_arrow_model = loadModel("EngineContent/Arrow.fbx");
    engine_handler_arrow_model->initializeVertexArrays();

    //load handler shader
    auto* handler_red = new ShaderProgram();
    handler_red->loadFromFile("EngineContent/Shader/HandlerRed.glsl");
    handler_red->compileShader();

    auto* handler_green = new ShaderProgram();
    handler_green->loadFromFile("EngineContent/Shader/HandlerGreen.glsl");
    handler_green->compileShader();

    auto* handler_blue = new ShaderProgram();
    handler_blue->loadFromFile("EngineContent/Shader/HandlerBlue.glsl");
    handler_blue->compileShader();

    std::string engine_collider_tag = "ENGINE_COLLIDER";
    std::string engine_handle_collider_tag = "ENGINE_HANDLE_COLLIDER";

    arrow_x = new Mesh3D(this,engine_handler_arrow_model);
    arrow_x->materials.push_back(handler_red);
    arrow_x->setRotationLocalDegrees(0, 90, 0);

    arrow_y = new Mesh3D(this,engine_handler_arrow_model);
    arrow_y->materials.push_back(handler_green);
    arrow_y->setRotationLocalDegrees(-90, 0, 0);

    arrow_z = new Mesh3D(this,engine_handler_arrow_model);
    arrow_z->materials.push_back(handler_blue);

    arrow_x_collider_ = dynamic_cast<Collider*>(arrow_x->get_child_by_tag(&engine_collider_tag));
    arrow_y_collider_ = dynamic_cast<Collider*>(arrow_y->get_child_by_tag(&engine_collider_tag));
    arrow_z_collider_ = dynamic_cast<Collider*>(arrow_z->get_child_by_tag(&engine_collider_tag));

    arrow_x_collider_->remove_tag(engine_collider_tag);
    arrow_y_collider_->remove_tag(engine_collider_tag);
    arrow_z_collider_->remove_tag(engine_collider_tag);

    arrow_x_collider_->add_tag(engine_handle_collider_tag);
    arrow_y_collider_->add_tag(engine_handle_collider_tag);
    arrow_z_collider_->add_tag(engine_handle_collider_tag);
    
    detach();

    IGNORE_IN_SCENE_TREE_VIEW = true;
}

void Handle::attach_to_object(Object3D* object_3d)
{
    visible = true;
    attached_ = true;
    attached_object_3d_ = object_3d;
    this->setPositionLocal(object_3d->getWorldPosition());
}

void Handle::detach()
{
    visible = false;
    attached_ = false;
    attached_object_3d_ = nullptr;
    handler_status = not_transforming;
}

bool Handle::is_attached() const
{
    return attached_;
}

bool Handle::is_moving_coord() const
{
    return handler_status != not_transforming;
}

void Handle::editor_click_handle(glm::vec3 camera_pos, glm::vec3 ray_direction)
{
    RayCastHit cast_hit = RayCastHit{
        false,
        100000 + 1.0,
        nullptr,
        glm::vec3(),
        glm::vec3()
    };
    arrow_x_collider_->check_collision(camera_pos, ray_direction, 100000.0, true, &cast_hit);
    if (cast_hit.hit)
    {
        std::cout << "hitx";
        handler_status = move_global_x;
        return;
    }

    arrow_y_collider_->check_collision(camera_pos, ray_direction, 100000.0, true, &cast_hit);
    if (cast_hit.hit)
    {
        std::cout << "hity";
        handler_status = move_global_y;
        return;
    }


    arrow_z_collider_->check_collision(camera_pos, ray_direction, 100000.0, true, &cast_hit);
    if (cast_hit.hit)
    {
        std::cout << "hitz";
        handler_status = move_global_z;
    }
}

void Handle::editor_release_handle()
{
    handler_status = not_transforming;
}

void Handle::editor_move_handle(glm::vec3 camera_pos, glm::vec3 ray_direction)
{
    Intersection *intersection = new Intersection;
    
    switch (handler_status)
    {
    case not_transforming:
        return;
    case move_global_x:
        RayIntersectionHelper::RayPlaneIntersection(intersection,camera_pos,ray_direction,attached_object_3d_->getWorldPosition(),vecZ);
        if (intersection->intersected)
        {
            auto current_object_pos = attached_object_3d_->getWorldPosition();
            current_object_pos.x = intersection->intersection_point.x;
            attached_object_3d_->set_position_global(current_object_pos);
            this->set_position_global(current_object_pos);
        }
        return;
    case move_global_y:
        RayIntersectionHelper::RayPlaneIntersection(intersection,camera_pos,ray_direction,attached_object_3d_->getWorldPosition(),vecZ);
        if (intersection->intersected)
        {
            auto current_object_pos = attached_object_3d_->getWorldPosition();
            current_object_pos.y = intersection->intersection_point.y;
            attached_object_3d_->set_position_global(current_object_pos);
            this->set_position_global(current_object_pos);

        }
        return;
    case move_global_z:
        RayIntersectionHelper::RayPlaneIntersection(intersection,camera_pos,ray_direction,attached_object_3d_->getWorldPosition(),vecX);
        if (intersection->intersected)
        {
            auto current_object_pos = attached_object_3d_->getWorldPosition();
            current_object_pos.z = intersection->intersection_point.z;
            attached_object_3d_->set_position_global(current_object_pos);
            this->set_position_global(current_object_pos);

        }
        return;
    }
    
    free(intersection);
}
