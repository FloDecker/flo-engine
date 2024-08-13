#pragma once
#include <vec2.hpp>
#include <vector>

#include "AbstractVoxelizer.h"
#include "../../Object3D.h"
#include "../../SceneContext.h"

class Voxelizer : public AbstractVoxelizer
{
public:
    explicit Voxelizer(GlobalContext* global_context, SceneContext* scene_context);
    void recalculate() override;
    void load_into_voxel_texture(Texture3D* texture_3d) override;
    void load_into_voxel_texture_df(Texture3D* texture_3d);

private:
    std::vector<glm::vec3> voxel_positions_; //contains the coordinates of the zero level set in ws coordiantes 
	std::vector<glm::i16vec3> zero_level_set; //contains the index of the zero level set in


	void calculate_area_filled_by_polygons(SceneContext* scene_context);
    void calculate_area_filled_recursive(SceneContext* scene_context, glm::vec3 ws_upper_right, glm::vec3 ws_lower_left,
                                         glm::i16vec3 voxel_upper_right, glm::i16vec3 voxel_lower_left);

protected:
};
