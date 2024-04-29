#pragma once
#include <vec2.hpp>
#include <vector>

#include "../Object3D.h"
#include "../SceneContext.h"

class Voxelizer : public Object3D
{
public:
    explicit Voxelizer(GlobalContext* global_context, SceneContext* scene_context);
    bool show_voxels = true;
    bool show_bounds;
    int voxel_precision;
    void recalculate();
private:
    std::vector<glm::vec3> voxel_positions_;
    SceneContext *scene_context_;
protected:
    
};
