#pragma once
#include "Mesh3D.h"
#include "../../Util/AssetLoader.h"

class Handle : public Object3D
{
public:
    explicit Handle(GlobalContext* global_context, Object3D* root);
    void attach_to_object(Object3D* object_3d);
    void detach();
    bool is_attached() const;

private:
    Object3D* attached_object_3d_;
    bool attached_ = false;
    bool handle_mode_global_ = true;
};
