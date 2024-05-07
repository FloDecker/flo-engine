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
    SceneContext* scene_context_;
    bool proble_single_chunc(SceneContext* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
                             float overlap_percentage = 0.1f);
    void calculate_area_filled_recursive(SceneContext* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
                                         glm::i16vec3 voxel_upper_right, glm::i16vec3 voxel_lower_left);

protected:
};
