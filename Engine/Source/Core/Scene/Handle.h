#pragma once
#include "Mesh3D.h"
#include "../../Util/AssetLoader.h"

class Collider;

enum HandlerType
{
    not_transforming,
    move_global_x,
    move_global_y,
    move_global_z
};

class Handle : public Object3D
{
public:
    explicit Handle(Scene *scene);
    void attach_to_object(Object3D* object_3d);
    void detach();
    bool is_attached() const;
    bool is_moving_coord() const;
    void editor_click_handle(glm::vec3 camera_pos, glm::vec3 ray_direction);
    void editor_release_handle();
    void editor_move_handle(glm::vec3 camera_pos, glm::vec3 ray_direction);
private:
    Mesh3D *arrow_x;
    Mesh3D *arrow_y;
    Mesh3D *arrow_z;

    Collider *arrow_x_collider_;
    Collider *arrow_y_collider_;
    Collider *arrow_z_collider_;
    Object3D* attached_object_3d_;
    bool attached_ = false;
    bool handle_mode_global_ = true;
    HandlerType handler_status = not_transforming;
    //-1 not attached
    //0 = x
    //1 = y
    //2 = z
    
};
