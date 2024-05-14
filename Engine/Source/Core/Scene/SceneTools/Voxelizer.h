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
    void load_into_voxel_texture(Texture3D* texture_3d);

private:
    std::vector<glm::vec3> voxel_positions_;
    SceneContext* scene_context_;
    void calculate_area_filled_recursive(SceneContext* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
                                         glm::i16vec3 voxel_upper_right, glm::i16vec3 voxel_lower_left);

protected:
};
